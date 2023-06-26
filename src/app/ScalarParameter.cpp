#include "ScalarParameter.h"

#include <algorithm>

ScalarParameter::ScalarParameter(const std::string& name, const Scalar initial,
                                 const Scalar min, const Scalar max)
    : m_name(name), m_value(initial), m_min(min), m_max(max), m_isFixed(false) {}

const std::string& ScalarParameter::name() const { return m_name; }

Scalar ScalarParameter::value() const { return m_value; }

void ScalarParameter::setValue(Scalar value) {
    value = std::clamp(value, m_min, m_max);
    if (!fuzzyEquals(m_value, value)) {
        m_value = value;
        m_isFixed = false;
        valueChanged(m_name, value);
    }
}

Scalar ScalarParameter::min() const { return m_min; }

void ScalarParameter::setMin(const Scalar min) {
    if (!fuzzyEquals(m_min, min)) {
        m_min = min;
        m_isFixed = false;
        enforceBounds();
    }
}

Scalar ScalarParameter::max() const { return m_max; }

void ScalarParameter::setMax(const Scalar max) {
    if (!fuzzyEquals(m_max, max)) {
        m_max = max;
        m_isFixed = false;
        enforceBounds();
    }
}

void ScalarParameter::enforceBounds() {
    if (m_value < m_min) {
        m_value = m_min;
        valueChanged(m_name, m_min);
    } else if (m_value > m_max) {
        m_value = m_max;
        valueChanged(m_name, m_max);
    }
}

bool ScalarParameter::isFixed() const { return m_isFixed; }

void ScalarParameter::setFixed(const Scalar value) {
    setMin(value);
    setMax(value);
    setValue(value);
    m_isFixed = true;
}

std::shared_ptr<nativeformat::param::Param> ScalarParameter::createParamFrom() {
    return nativeformat::param::createParam(value(), max(), min(), name());
}