#include "NewtonCotes.h"

#include <cmath>

inline double relativeError(const double x1, const double x0) {
    return fabs(x1 / x0 - 1);
}

NewtonCotes::NewtonCotes(const int rate, const Type type, const double precision,
                         const int maxIterations)
    : m_rate(rate),
      m_type(type),
      m_precision(precision),
      m_maxIterations(maxIterations),
      m_nAbcissas(1) {}

double NewtonCotes::integrate(const UnivariateRealFunction& f, const double a,
                              const double b) {
    double sum0, sum1(std::numeric_limits<double>::quiet_NaN());
    for (int iter = 1; iter <= m_maxIterations; ++iter) {
        sum0 = sum1;
        sum1 = next(iter, f, a, b, sum0);

        if ((iter > 3) && relativeError(sum1, sum0) < m_precision) {
            break;  // converged
        }
    }

    return sum1;
}

double NewtonCotes::next(const int iter, const UnivariateRealFunction& f, const double a,
                         const double b, const double sum0) {
    double sum = sum0;

    if (iter == 1) {
        m_h = (b - a);
        m_nAbcissas = 1;

        switch (m_type) {
            case CLOSED:  // use the two endpoints
                sum = 0.5 * m_h * (f(a) + f(b));
                break;
            case OPEN:  // use the midpoint
                sum = m_h * (f((a + b) / 2));
                break;
        }
        return sum;
    } else {
        // summing up the interior abscissas
        m_h /= m_rate;
        m_nAbcissas *= m_rate;

        double addition = 0;
        double x = a;
        for (int i = 1; i < m_nAbcissas; ++i) {
            x += m_h;
            if ((i % m_rate) == 0) {
                continue;  // this abscissa is already computed in a previous
                           // iteration
            }

            addition += f(x);
        }

        sum = sum0 / m_rate + m_h * addition;
    }

    return sum;
}
