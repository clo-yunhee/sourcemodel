#include "Butterworth.h"

#include <boost/math/constants/constants.hpp>
#include <complex>

using namespace boost::math::double_constants;

SOSFilter butterworth::lowPass(const int N, const double fc, const double fs) {
    const double Wn = fc / (fs / 2.0);
    const double Wo = std::tan((pi * fc) / fs);

    std::vector<std::complex<double>> p;

    // Step 1. Get Butterworth analog lowpass prototype.
    for (int i = 2 + N - 1; i <= 3 * N - 1; i += 2) {
        p.push_back(std::polar<double>(1, (half_pi * i) / N));
    }

    // Step 2. Transform to low pass filter.
    std::complex<double> Sg = 1.0, prodSp = 1.0;

    std::vector<std::complex<double>> Sp(p.size()), Sz(0);

    for (int i = 0; i < p.size(); ++i) {
        Sg *= Wo;
        Sp[i] = Wo * p[i];
        prodSp *= (1.0 - Sp[i]);
    }

    // Step 3. Transform to digital filter.
    std::vector<std::complex<double>> P(Sp.size()), Z(Sp.size(), -1);

    double G = std::real(Sg / prodSp);

    for (int i = 0; i < Sp.size(); ++i) {
        P[i] = (1.0 + Sp[i]) / (1.0 - Sp[i]);
    }

    // Step 6. Convert to SOS.

    return {Z, P, G};
}

SOSFilter butterworth::highPass(int N, const double fc, const double fs) {
    const double Wn = fc / (fs / 2.0);
    const double Wo = std::tan((pi * fc) / fs);

    std::vector<std::complex<double>> p;

    // Step 1. Get Butterworth analog lowpass prototype.
    for (int i = 2 + N - 1; i <= 3 * N - 1; i += 2) {
        p.push_back(std::polar<double>(1, (half_pi * i) / N));
    }

    // Step 2. Transform to high pass filter.
    std::complex<double> Sg = 1.0, prodSp = 1.0, prodSz = 1.0;

    std::vector<std::complex<double>> Sp(p.size()), Sz(p.size());

    for (int i = 0; i < p.size(); ++i) {
        Sg *= -p[i];
        Sp[i] = Wo / p[i];
        Sz[i] = 0.0;
        prodSp *= (1.0 - Sp[i]);
        prodSz *= (1.0 - Sz[i]);
    }
    Sg = 1.0 / Sg;

    // Step 3. Transform to digital filter.
    std::vector<std::complex<double>> P(Sp.size()), Z(Sp.size());

    double G = std::real(Sg * prodSz / prodSp);

    for (int i = 0; i < Sp.size(); ++i) {
        P[i] = (1.0 + Sp[i]) / (1.0 - Sp[i]);
        Z[i] = (1.0 + Sz[i]) / (1.0 - Sz[i]);
    }

    // Step 6. Convert to SOS.

    return {Z, P, G};
}