#include "SOSFilter.h"

SOSFilter::SOSFilter(const std::vector<std::array<Scalar, 6>>& sos)
    : m_sos(sos), m_zi(sos.size(), {0_f, 0_f}) {}

SOSFilter::SOSFilter(const std::vector<std::complex<Scalar>>& z,
                     const std::vector<std::complex<Scalar>>& p, const Scalar k)
    : SOSFilter(zpk2sos(z, p, k)) {}

std::vector<Scalar> SOSFilter::filter(const std::vector<Scalar>& x) {
    std::vector<Scalar> y(x.size());

    for (int k = 0; k < x.size(); ++k) {
        Scalar x_cur = x[k];
        Scalar x_new;
        for (int s = 0; s < m_sos.size(); ++s) {
            x_new = m_sos[s][0] * x_cur + m_zi[s][0];
            m_zi[s][0] = m_sos[s][1] * x_cur - m_sos[s][4] * x_new + m_zi[s][1];
            m_zi[s][1] = m_sos[s][2] * x_cur - m_sos[s][5] * x_new;
            x_cur = x_new;
        }
        y[k] = x_cur;
    }

    return y;
}

const std::vector<std::array<Scalar, 6>>& SOSFilter::coefficients() const {
    return m_sos;
}