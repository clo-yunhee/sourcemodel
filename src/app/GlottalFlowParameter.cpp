#include "GlottalFlowParameter.h"

#include <algorithm>

#include "math/utils.h"

GlottalFlowParameter::GlottalFlowParameter(const double initial, const double min,
                                           const double max)
    : m_value(initial), m_min(min), m_max(max), m_isFixed(false) {}

void GlottalFlowParameter::disconnectAll() {
    valueChanged.disconnect_all();
    minChanged.disconnect_all();
    maxChanged.disconnect_all();
}

double GlottalFlowParameter::value() const { return m_value; }

void GlottalFlowParameter::setValue(double value) {
    value = std::clamp(value, m_min, m_max);
    if (!fuzzyEquals(m_value, value)) {
        m_value = value;
        m_isFixed = false;
        valueChanged(value);
    }
}

double GlottalFlowParameter::min() const { return m_min; }

void GlottalFlowParameter::setMin(const double min) {
    if (!fuzzyEquals(m_min, min)) {
        m_min = min;
        m_isFixed = false;
        minChanged(min);
        enforceBounds();
    }
}

double GlottalFlowParameter::max() const { return m_max; }

void GlottalFlowParameter::setMax(const double max) {
    if (!fuzzyEquals(m_max, max)) {
        m_max = max;
        m_isFixed = false;
        maxChanged(max);
        enforceBounds();
    }
}

void GlottalFlowParameter::enforceBounds() {
    if (m_value < m_min) {
        m_value = m_min;
        valueChanged(m_min);
    } else if (m_value > m_max) {
        m_value = m_max;
        valueChanged(m_max);
    }
}

bool GlottalFlowParameter::isFixed() const { return m_isFixed; }

void GlottalFlowParameter::setFixed(const double value) {
    setMin(value);
    setMax(value);
    setValue(value);
    m_isFixed = true;
}