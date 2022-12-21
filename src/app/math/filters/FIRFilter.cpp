#include "FIRFilter.h"

FIRFilter::FIRFilter(const std::vector<double>& coefs)
    : m_coefs(coefs), m_memory(coefs.size() - 1, 0.0) {
    m_memory.set_capacity(coefs.size() - 1);
}

std::vector<double> FIRFilter::filter(const std::vector<double>& x) {
    std::vector<double> y(x.size());

    for (int k = 0; k < x.size(); ++k) {
        y[k] = m_coefs[0] * x[k];
        for (int j = 1; j < m_coefs.size(); ++j) {
            y[k] += m_coefs[j] * m_memory[j - 1];
        }
        m_memory.push_back(x[k]);
    }

    return y;
}