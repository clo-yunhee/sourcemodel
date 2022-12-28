#include "BufferedGenerator.h"

#include <algorithm>

#include "audio/AudioTime.h"

BufferedGenerator::BufferedGenerator(const AudioTime& time)
    : m_time(time),
      m_bufferLength(1024),
      m_buffer(1024, 0.0),
      m_fs(48000),
      m_fsChanged(true),
      m_isNormalized(true) {
    m_buffer.set_capacity(1024);

    m_gainReductionComputer.setThreshold(-10.0f);
    m_gainReductionComputer.setKnee(0.0f);
    m_gainReductionComputer.setAttackTime(30.0f / 1000);
    m_gainReductionComputer.setReleaseTime(100.0f / 1000);
    m_gainReductionComputer.setRatio(std::numeric_limits<float>::infinity());
    m_gainReductionComputer.setMakeUpGain(0.0f);

    m_lookAheadGainReduction.setDelayTime(5.0f / 1000);
}

void BufferedGenerator::setBufferLength(const int bufferLength) {
    if (m_bufferLength != bufferLength) {
        std::unique_lock lock(m_mutex);

        m_bufferLength = bufferLength;
        m_buffer.resize(bufferLength);
        m_buffer.set_capacity(bufferLength);
    }
}

void BufferedGenerator::copyBufferTo(std::vector<double>& out) {
    std::shared_lock lock(m_mutex);

    std::copy(m_buffer.begin(), m_buffer.end(), out.begin());
}

void BufferedGenerator::fillBuffer(std::vector<double>& out) {
    // Backup if true it'll get set to false by fillInternalBuffer.
    const bool wasSampleRateChanged = m_fsChanged;

    m_tempBuffer.resize(out.size());
    fillInternalBuffer(m_tempBuffer);

    if (wasSampleRateChanged) {
        // Re-prepare the gain reduction stuff.
        m_gainReductionComputer.prepare(m_fs);
        m_lookAheadGainReduction.prepare(m_fs, out.size());
    }

    processGainReduction();

    // Copy into output and ring buffer.
    std::copy(m_tempBuffer.begin(), m_tempBuffer.end(), out.begin());
    {
        std::unique_lock lock(m_mutex);
        m_buffer.insert(m_buffer.end(), out.begin(), out.end());
    }
}

void BufferedGenerator::setSampleRate(const double fs) {
    m_fs = fs;
    m_fsChanged = true;
}

double BufferedGenerator::sampleRate() const { return m_fs; }

void BufferedGenerator::setNormalized(const bool isNorm) {
    m_gainReductionComputer.setRatio(isNorm ? std::numeric_limits<float>::infinity() : 2);
    m_isNormalized = isNorm;
}

bool BufferedGenerator::isNormalized() const { return m_isNormalized; }

double BufferedGenerator::fs() const { return m_fs; }

bool BufferedGenerator::hasSampleRateChanged() const { return m_fsChanged; }

void BufferedGenerator::ackSampleRateChange() { m_fsChanged = false; }

double BufferedGenerator::time(const int off) const { return m_time.time(off); }

void BufferedGenerator::processGainReduction() {
    const int length = m_tempBuffer.size();

    m_sidechainSignal.resize(length);
    m_gainReduction.resize(length);

    // Sidechain signal (abs of signal)
    for (int i = 0; i < length; ++i) {
        m_sidechainSignal[i] = fabs(m_tempBuffer[i]);
    }

    // Compute gain reduction samples
    m_gainReductionComputer.computeGainInDecibelsFromSidechainSignal(
        m_sidechainSignal.data(), m_sidechainSignal.data(), length);

    m_lookAheadGainReduction.pushSamples(m_sidechainSignal.data(), length);
    m_lookAheadGainReduction.process();
    m_lookAheadGainReduction.readSamples(m_sidechainSignal.data(), length);

    const double makeUpGainInDecibels = m_gainReductionComputer.getMakeUpGain();
    for (int i = 0; i < length; ++i) {
        m_sidechainSignal[i] =
            pow(10, (makeUpGainInDecibels + m_sidechainSignal[i]) / 10);
        if (std::isinf(m_sidechainSignal[i]) || std::isnan(m_sidechainSignal[i])) {
            fprintf(stderr, "blep\n");
        }
    }

    for (int i = 0; i < length; ++i) {
        m_tempBuffer[i] *= m_sidechainSignal[i];
    }
}