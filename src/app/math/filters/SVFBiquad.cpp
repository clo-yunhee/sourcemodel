#include "SVFBiquad.h"

#include <complex>
#include <iostream>

void SVFBiquad::update(const double b0, const double b1, const double b2, const double a1,
                       const double a2) {
    using namespace std::complex_literals;

    const double m1 = -1 - a1 - a2;
    const double m2 = -1 + a1 - a2;

    const std::complex<double> sm1 = sqrt(m1 + 0.0i);
    const std::complex<double> sm2 = sqrt(m2 + 0.0i);

    const double sm1div2 = (sm1 / sm2).real();
    const double sm1mul2 = (sm1 * sm2).real();

    _g = sm1div2;
    _R = (a2 - 1) / sm1mul2;
    _cHP = (b0 - b1 + b2) / (1 - a1 + a2);
    _cBP = (2 * (b0 - b2)) / sm1mul2;
    _cLP = (b0 + b1 + b2) / (1 + a1 + a2);

    _HP.update(_g, _R, SVFPiece::FltType_HIGHPASS);
    _BP.update(_g, _R, SVFPiece::FltType_BANDPASS);
    _LP.update(_g, _R, SVFPiece::FltType_LOWPASS);
}

double SVFBiquad::tick(const double x) {
    return _cHP * _HP.tick(x) + _cBP * _BP.tick(x) + _cLP * _LP.tick(x);
}