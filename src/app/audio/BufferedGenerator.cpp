#include "BufferedGenerator.h"

#include "audio/AudioTime.h"

BufferedGenerator::BufferedGenerator(const AudioTime& time)
    : m_time(time), m_bufferLength(1024), m_buffer(1024, 0.0) {
    m_buffer.set_capacity(1024);
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
    std::unique_lock lock(m_mutex);

    std::vector<double> out2(out.size());
    fillInternalBuffer(out2);
    std::copy(out2.begin(), out2.end(), out.begin());

    m_buffer.insert(m_buffer.end(), out.begin(), out.end());
}

double BufferedGenerator::time(const int off) const { return m_time.time(off); }