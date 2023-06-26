#include "ToggleParameter.h"

#include <algorithm>

ToggleParameter::ToggleParameter(const std::string& name, const bool initial)
    : m_name(name), m_value(initial) {}

const std::string& ToggleParameter::name() const { return m_name; }

bool ToggleParameter::value() const { return m_value; }

void ToggleParameter::setValue(const bool value) {
    if (m_value != value) {
        m_value = value;
        valueChanged(m_name, value ? 1.0_f : 0.0_f);
    }
}
