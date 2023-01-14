#include "Butterworth.h"

#include <algorithm>
#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/cos_pi.hpp>
#include <boost/math/special_functions/sin_pi.hpp>
#include <iostream>

using namespace boost::math::constants;
using boost::math::cos_pi;
using boost::math::sin_pi;

Butterworth::Butterworth() {}

bool Butterworth::loPass(const Scalar fs, const Scalar fc, const int order) {
    return coefficients(kLoPass, fs, fc, fc, order);
}

bool Butterworth::hiPass(const Scalar fs, const Scalar fc, const int order) {
    return coefficients(kHiPass, fs, fc, fc, order);
}

bool Butterworth::bandPass(const Scalar fs, const Scalar f1c, const Scalar f2c,
                           const int order) {
    return coefficients(kBandPass, fs, f1c, f2c, order);
}

bool Butterworth::bandStop(const Scalar fs, const Scalar f1c, const Scalar f2c,
                           const int order) {
    return coefficients(kBandStop, fs, f1c, f2c, order);
}

bool Butterworth::coefficients(const FilterType filterType, const Scalar fs,
                               const Scalar _f1c, const Scalar _f2c,
                               const int filterOrder) {
    // ** Pre-warping
    Scalar f1c = 2 * std::tan(pi<Scalar>() * _f1c / fs);
    Scalar f2c = 2 * std::tan(pi<Scalar>() * _f2c / fs);

    // ** Design basic S-plane poles-only analogue LP prototype
    std::vector<dcomplex> poles(2 * filterOrder);

    for (int k = 0; k < filterOrder; ++k) {
        const Scalar arg = Scalar(2 * k + 1) / Scalar(2 * filterOrder);
        const Scalar real = -sin_pi(arg);
        const Scalar imag = cos_pi(arg);
        poles[2 * k].real(real);
        poles[2 * k].imag(imag);
        poles[2 * k + 1].real(real);
        poles[2 * k + 1].imag(-imag);  // Conjugate.
    }

    std::vector<dcomplex> zeros(0);  // Butterworth LP prototype has no zeros.
    Scalar gain = 1.0_f;             // Always 1 for the Butterworth prototype lowpass.

    // ** Convert prototype to target filter type (LP/HP/BP/BS) - S-plane

    // Re-orient BP/BS corner frequencies if necessary
    if (f1c > f2c) {
        std::swap(f1c, f2c);
    }

    switch (filterType) {
        case kLoPass:
            convertToLoPass(f2c, zeros, poles, gain);
            break;
        case kHiPass:
            convertToHiPass(f2c, zeros, poles, gain);
            break;
        case kBandPass:
            convertToBandPass(sqrt(f1c * f2c), f2c - f1c, zeros, poles, gain);
            break;
        case kBandStop:
            convertToBandStop(sqrt(f1c * f2c), f2c - f1c, zeros, poles, gain);
            break;
        default:
            std::cerr << "Butterworth: unknown filter type" << std::endl;
            return false;
    }

    // ** SANITY CHECK: Ensure poles are in the left half of the S-plane
    for (const auto& p : poles) {
        if (p.real() > 0) {
            std::cerr << "Butterworth: poles must be in the left half plane" << std::endl;
            return false;
        }
    }

    // ** Map zeros & poles from S-plane to Z-plane
    const Scalar preBltGain = gain;

    for (auto& z : zeros) {
        gain /= bilinear(z);
    }

    for (auto& p : poles) {
        gain *= bilinear(p);
    }

    // ** Split up Z-plane poles & zeros into SOS
    m_sos = zp2sos(zeros, poles);

    // Correct the overall gain
    if (filterType == kLoPass ||
        filterType == kBandPass) {                // pre-BLT is okay for S-plane
        gain = preBltGain * (preBltGain / gain);  // 2nd term is how much BLT boosts
    } else if (filterType == kHiPass || filterType == kBandStop) {  // HF gain != DC gain
        gain = 1 / gain;
    }

    for (int k = 0; k < m_sos.size(); ++k) {
        for (int i = 0; i < 3; ++i) {
            m_sos[k][i] *= pow(gain, 1 / m_sos.size());
        }
    }

    m_zi.resize(m_sos.size(), {0, 0});

    return true;
}

void Butterworth::convertToLoPass(const Scalar Wc, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, Scalar& gain) {
    gain *= pow(Wc, poles.size());

    zeros.resize(0);         // Poles only
    for (auto& p : poles) {  // Scale poles by the cutoff Wc
        p = Wc * p;
    }
}

void Butterworth::convertToHiPass(const Scalar Wc, std::vector<dcomplex>& zeros,
                                  std::vector<dcomplex>& poles, Scalar& gain) {
    // : hp_S = Wc / lp_S

    // Calculate gain
    dcomplex prodz = 1.0_f;
    dcomplex prodp = 1.0_f;

    for (const auto& z : zeros) prodz *= -z;
    for (const auto& p : poles) prodp *= -p;

    gain *= prodz.real() / prodp.real();

    // Convert LP poles to HP
    for (auto& p : poles) {
        if (std::abs(p) != 0) {
            p = Wc / p;
        }
    }

    // Init with zeros, no non-zero values to convert
    zeros.resize(poles.size());
    for (auto& z : zeros) {
        z = 0;
    }
}

void Butterworth::convertToBandPass(const Scalar Wc, const Scalar bw,
                                    std::vector<dcomplex>& zeros,
                                    std::vector<dcomplex>& poles, Scalar& gain) {
    // : bp_S = 0.5 * lp_S * BW + 0.5 * sqrt(BW^2 * lp_S^2 - 4*Wc^2)

    // Calculate bandpass gain
    gain *= pow(bw, poles.size() - zeros.size());

    const int numPoles = poles.size();

    // Convert LP poles to BP in an ordered list of poles and their complex conjugates
    poles.resize(2 * numPoles);

    for (int i = 0; i < numPoles; ++i) {
        const auto& p = poles[i];
        if (std::abs(p) != 0) {
            const dcomplex term1 = 0.5_f * p * bw;
            const dcomplex term2 = 0.5_f * std::sqrt((bw * bw) * (p * p) - (4 * Wc * Wc));
            poles[i] = term1 + term2;
            poles[numPoles + i] = term1 - term2;
        }
    }

    // Init zeros, no non-zero values to convert
    zeros.resize(poles.size());
    for (auto& z : zeros) {
        z = 0;
    }
}

void Butterworth::convertToBandStop(const Scalar Wc, const Scalar bw,
                                    std::vector<dcomplex>& zeros,
                                    std::vector<dcomplex>& poles, Scalar& gain) {
    // : bs_S = 0.5 * BW / lp_S + 0.5 * sqrt(BW^2 / lp_S^2 - 4*Wc^2)

    // Calculate gain
    dcomplex prodz = 1.0_f;
    dcomplex prodp = 1.0_f;

    for (const auto& z : zeros) prodz *= -z;
    for (const auto& p : poles) prodp *= -p;

    gain *= prodz.real() / prodp.real();

    const int numPoles = poles.size();

    // Convert LP zeros to band stop
    zeros.resize(2 * numPoles);

    for (int i = 0; i < numPoles; ++i) {
        zeros[2 * i].real(0);
        zeros[2 * i].imag(Wc);
        zeros[2 * i + 1].real(0);
        zeros[2 * i + 1].imag(-Wc);  // Complex conjugate
    }

    // Convert LP poles to band stop
    poles.resize(2 * numPoles);

    for (int i = 0; i < numPoles; ++i) {
        const auto& p = poles[i];
        if (std::abs(p) != 0) {
            const dcomplex term1 = 0.5_f * bw / p;
            const dcomplex term2 = 0.5_f * std::sqrt((bw * bw) / (p * p) - (4 * Wc * Wc));
            poles[i] = term1 + term2;
            poles[numPoles + i] = term1 - term2;
        }
    }
}

Scalar Butterworth::bilinear(dcomplex& sz) {
    dcomplex s = sz;
    sz = (2.0_f + s) / (2.0_f - s);
    // Return the gain
    return std::abs(2.0_f - s);
}

std::vector<std::array<Scalar, 6>> Butterworth::zp2sos(
    const std::vector<dcomplex>& zeros, const std::vector<dcomplex>& poles) {
    const int numZeros = zeros.size();
    const int numPoles = poles.size();

    const int filterOrder = std::max(numZeros, numPoles);

    // Add zeros at -1, so if S-plane degenerate case
    // where numZeros = 0 will map to -1 in Z-plane.
    std::vector<dcomplex> z(filterOrder, -1.0_f);
    std::vector<dcomplex> p(filterOrder, 0.0_f);

    // Copy
    std::copy(zeros.begin(), zeros.end(), z.begin());
    std::copy(poles.begin(), poles.end(), p.begin());

    std::vector<std::array<Scalar, 6>> sos;

    for (int i = 0; i + 1 < filterOrder; i += 2) {
        sos.push_back({
            1,
            -(z[i] + z[i + 1]).real(),
            (z[i] * z[i + 1]).real(),
            1,
            -(p[i] + p[i + 1]).real(),
            (p[i] * p[i + 1]).real(),
        });
    }

    // Odd filter order thus one pair of poles/zeros remains
    if (filterOrder % 2 == 1) {
        sos.push_back({
            1,
            -z[filterOrder - 1].real(),
            0,
            1,
            -p[filterOrder - 1].real(),
            0,
        });
    }

    return sos;
}