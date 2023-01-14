#ifndef SOURCEMODEL__MATH_FILTERS_SOSFILTER_H
#define SOURCEMODEL__MATH_FILTERS_SOSFILTER_H

#include <array>
#include <complex>
#include <vector>

#include "math/utils.h"

class SOSFilter {
   public:
    SOSFilter(const std::vector<std::array<Scalar, 6>>& sos = {});

    // zpk2sos
    SOSFilter(const std::vector<std::complex<Scalar>>& z,
              const std::vector<std::complex<Scalar>>& p, Scalar k);

    std::vector<Scalar> filter(const std::vector<Scalar>& x);

    const std::vector<std::array<Scalar, 6>>& coefficients() const;

   protected:
    std::vector<std::array<Scalar, 6>> m_sos;
    std::vector<std::array<Scalar, 2>> m_zi;
};

std::vector<std::array<Scalar, 6>> zpk2sos(const std::vector<std::complex<Scalar>>& z,
                                           const std::vector<std::complex<Scalar>>& p,
                                           Scalar                                   k);

#endif  // SOURCEMODEL__MATH_FILTERS_SOSFILTER_H