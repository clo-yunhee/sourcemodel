#include "BufferedGenerator.h"

#include <algorithm>
#include <iostream>

#include "audio/AudioTime.h"

BufferedGenerator::BufferedGenerator(const AudioTime& time)
    : m_time(time),
      m_bufferLength(1024),
      m_buffer(1024, 0),
      m_fs(48000),
      m_fsChanged(false),
      m_isNormalized(true) {
    m_buffer.set_capacity(1024);

    m_gainReductionComputer.setThreshold(10.0f);
    m_gainReductionComputer.setKnee(0.0f);
    m_gainReductionComputer.setAttackTime(10.0f / 1000);
    m_gainReductionComputer.setReleaseTime(60.0f / 1000);
    m_gainReductionComputer.setRatio(std::numeric_limits<float>::infinity());
    m_gainReductionComputer.setMakeUpGain(0.0f);

    m_lookAheadGainReduction.setDelayTime(5.0f / 1000);
}

void BufferedGenerator::setBufferLength(const int bufferLength) {
    if (m_bufferLength != bufferLength) {
        std::unique_lock lock(m_mutex);

        m_bufferLength = bufferLength;
        m_buffer.rset_capacity(bufferLength);
        m_buffer.rresize(bufferLength);
    }
}

uint64_t BufferedGenerator::copyBufferTo(std::vector<Scalar>& out) {
    std::shared_lock lock(m_mutex);

    if (out.size() < m_bufferLength) {
        std::copy(std::prev(m_buffer.end(), out.size()), m_buffer.end(), out.begin());
    } else {
        std::copy(m_buffer.begin(), m_buffer.end(), out.begin());
    }
    return m_time.timeSamples(0);
}

bool BufferedGenerator::hasEnoughSamplesSince(uint64_t time, int count) {
    return (m_time.timeSamples(0) - time) >= count;
}

void BufferedGenerator::fillBuffer(std::vector<Scalar>& out) {
    // Backup if true it'll get set to false by fillInternalBuffer.
    const bool wasSampleRateChanged = m_fsChanged;

    m_internalBuffer.resize(out.size());
    fillInternalBuffer(m_internalBuffer);

    if (wasSampleRateChanged) {
        // Re-prepare the gain reduction stuff.
        m_gainReductionComputer.prepare(m_fs);
        m_lookAheadGainReduction.prepare(m_fs, out.size());
        // Resize the delay buffer.
        m_delaySamples = m_lookAheadGainReduction.getDelayInSamples();
        m_delayBuffer.rset_capacity(out.size() + m_delaySamples);
        m_delayBuffer.rresize(out.size() + m_delaySamples);
        std::fill(m_delayBuffer.begin(), std::next(m_delayBuffer.begin(), m_delaySamples),
                  0.0_f);
    }

    m_delayBuffer.insert(m_delayBuffer.end(), m_internalBuffer.begin(),
                         m_internalBuffer.end());

    if (m_isNormalized) {
        processGainReduction();
    }

    // Copy into output and ring buffer.
    std::copy(m_delayBuffer.begin(), std::next(m_delayBuffer.begin(), out.size()),
              out.begin());

    m_mutex.lock();
    m_buffer.insert(m_buffer.end(), out.begin(), out.end());
    m_mutex.unlock();
}

void BufferedGenerator::setSampleRate(const Scalar fs) {
    m_fs = fs;
    m_fsChanged = true;
}

Scalar BufferedGenerator::sampleRate() const { return m_fs; }

void BufferedGenerator::setNormalized(const bool isNorm) { m_isNormalized = isNorm; }

bool BufferedGenerator::isNormalized() const { return m_isNormalized; }

Scalar BufferedGenerator::fs() const { return m_fs; }

bool BufferedGenerator::hasSampleRateChanged() const { return m_fsChanged; }

void BufferedGenerator::ackSampleRateChange() { m_fsChanged = false; }

Scalar BufferedGenerator::time(const int off) const { return m_time.time(off); }

void BufferedGenerator::processGainReduction() {
    const int length = m_internalBuffer.size();

    m_sidechainSignal.resize(length);
    m_gainReduction.resize(length);

    // Sidechain signal (abs of signal)
    for (int i = 0; i < length; ++i) {
        m_sidechainSignal[i] = fabs(m_internalBuffer[i]);
    }

    // Compute gain reduction samples
    m_gainReductionComputer.computeGainInDecibelsFromSidechainSignal(
        m_sidechainSignal.data(), m_sidechainSignal.data(), length);

    m_lookAheadGainReduction.pushSamples(m_sidechainSignal.data(), length);
    m_lookAheadGainReduction.process();
    m_lookAheadGainReduction.readSamples(m_sidechainSignal.data(), length);

    const float makeUpGainInDecibels = m_gainReductionComputer.getMakeUpGain();
    for (int i = 0; i < length; ++i) {
        m_sidechainSignal[i] =
            powf(10.0f, (makeUpGainInDecibels + m_sidechainSignal[i]) / 10.0f);
    }

    for (int i = 0; i < length; ++i) {
        m_delayBuffer[i] *= m_sidechainSignal[i];
    }
}