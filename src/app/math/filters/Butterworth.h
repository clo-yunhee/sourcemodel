#ifndef SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H
#define SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H

#include <complex>

#include "SOSFilter.h"
#include "math/utils.h"

class Butterworth : public SOSFilter {
   public:
    using dcomplex = std::complex<Scalar>;

    enum FilterType {
        kLoPass,
        kHiPass,
        kBandPass,
        kBandStop,
    };

    Butterworth();

    bool loPass(Scalar fs, Scalar fc, int order);
    bool hiPass(Scalar fs, Scalar fc, int order);
    bool bandPass(Scalar fs, Scalar f1c, Scalar f2c, int order);
    bool bandStop(Scalar fs, Scalar f1c, Scalar f2c, int order);

   private:
    bool coefficients(FilterType type, Scalar fs, Scalar f1c, Scalar f2c,
                      int filterOrder);

    static void convertToLoPass(Scalar Wc, std::vector<dcomplex>& zeros,
                                std::vector<dcomplex>& poles, Scalar& gain);

    static void convertToHiPass(Scalar Wc, std::vector<dcomplex>& zeros,
                                std::vector<dcomplex>& poles, Scalar& gain);

    static void convertToBandPass(Scalar Wc, Scalar bw, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, Scalar& gain);

    static void convertToBandStop(Scalar Wc, Scalar bw, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, Scalar& gain);

    static Scalar bilinear(dcomplex& sz);

    static std::vector<std::array<Scalar, 6>> zp2sos(const std::vector<dcomplex>& zeros,
                                                     const std::vector<dcomplex>& poles);
};

#endif  // SOURCEMODEL__MATH_FILTERS_BUTTERWORTH_H