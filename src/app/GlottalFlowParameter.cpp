#include "GlottalFlowParameter.h"

#include <algorithm>

#include "math_utils.h"

GlottalFlowParameter::GlottalFlowParameter(const double initial, const double min,
                                           const double max)
    : m_value(initial), m_min(min), m_max(max) {}

void GlottalFlowParameter::disconnectAll() {
    valueChanged.disconnect_all();
    minChanged.disconnect_all();
    maxChanged.disconnect_all();
}

double GlottalFlowParameter::value() const { return m_value; }

void GlottalFlowParameter::setValue(const double value) {
    m_value = std::clamp(value, m_min, m_max);
    valueChanged(value);
}

double GlottalFlowParameter::min() const { return m_min; }

void GlottalFlowParameter::setMin(const double min) {
    m_min = min;
    minChanged(min);
    enforceBounds();
}

double GlottalFlowParameter::max() const { return m_max; }

void GlottalFlowParameter::setMax(const double max) {
    m_max = max;
    maxChanged(max);
    enforceBounds();
}

void GlottalFlowParameter::enforceBounds() {
    const double newValue = std::clamp(m_value, m_min, m_max);
    if (!fuzzyEquals(m_value, newValue)) {
        setValue(newValue);
    }
}