#ifndef SOURCEMODEL__MATH_FILTERS_SOSFILTER_H
#define SOURCEMODEL__MATH_FILTERS_SOSFILTER_H

#include <array>
#include <complex>
#include <vector>

class SOSFilter {
   public:
    SOSFilter(const std::vector<std::array<double, 6>>& sos = {});

    // zpk2sos
    SOSFilter(const std::vector<std::complex<double>>& z,
              const std::vector<std::complex<double>>& p, double k);

    std::vector<double> filter(const std::vector<double>& x);

   protected:
    std::vector<std::array<double, 6>> m_sos;
    std::vector<std::array<double, 2>> m_zi;
};

std::vector<std::array<double, 6>> zpk2sos(const std::vector<std::complex<double>>& z,
                                           const std::vector<std::complex<double>>& p,
                                           double                                   k);

#endif  // SOURCEMODEL__MATH_FILTERS_SOSFILTER_H