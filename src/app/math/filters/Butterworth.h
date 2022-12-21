#ifndef SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H
#define SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H

#include <complex>

#include "SOSFilter.h"

class Butterworth : public SOSFilter {
   public:
    using dcomplex = std::complex<double>;

    enum FilterType {
        kLoPass,
        kHiPass,
        kBandPass,
        kBandStop,
    };

    Butterworth();

    bool loPass(double fs, double fc, int order);
    bool hiPass(double fs, double fc, int order);
    bool bandPass(double fs, double f1c, double f2c, int order);
    bool bandStop(double fs, double f1c, double f2c, int order);

   private:
    bool coefficients(FilterType type, double fs, double f1c, double f2c,
                      int filterOrder);

    static void convertToLoPass(double Wc, std::vector<dcomplex>& zeros,
                                std::vector<dcomplex>& poles, double& gain);

    static void convertToHiPass(double Wc, std::vector<dcomplex>& zeros,
                                std::vector<dcomplex>& poles, double& gain);

    static void convertToBandPass(double Wc, double bw, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, double& gain);

    static void convertToBandStop(double Wc, double bw, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, double& gain);

    static double bilinear(dcomplex& sz);

    static std::vector<std::array<double, 6>> zp2sos(const std::vector<dcomplex>& zeros,
                                                     const std::vector<dcomplex>& poles);
};

#endif  // SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H