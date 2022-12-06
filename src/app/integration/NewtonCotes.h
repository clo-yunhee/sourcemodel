#ifndef SOURCEMODEL__INTEGRATION_NEWTON_COTES_H
#define SOURCEMODEL__INTEGRATION_NEWTON_COTES_H

#include <functional>

using UnivariateRealFunction = std::function<double(double)>;

class NewtonCotes {
   public:
    enum Type { CLOSED, OPEN };

    NewtonCotes(int rate, Type type, double precision, int maxIterations);

    double integrate(const UnivariateRealFunction& f, double a, double b);

    double next(int iter, const UnivariateRealFunction& f, double a, double b, double sum0);

   private:
    int    m_rate;
    Type   m_type;
    double m_precision;
    int    m_maxIterations;

    double m_h;
    int    m_nAbcissas;
};

#endif  // SOURCEMODEL__INTEGRATION_NEWTON_COTES_H