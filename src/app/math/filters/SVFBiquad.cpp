#include "SVFBiquad.h"

#include <iostream>

inline Scalar asqrt(Scalar x) { return std::sqrt(std::abs(x)); }

inline Scalar rcsqrtdiv(const bool m1pos, const bool m2pos, const Scalar asm1,
                        const Scalar asm2) {
    // asm1 = sqrt|m1|
    // asm2 = sqrt|m2|
    // <-- real(sqrt(m1) / sqrt(m2))

    // Real part is asm1/asm2 if same sign
    //              0         otherwise

    if (m1pos != m2pos) return 0;
    return asm1 / asm2;
}

inline Scalar rcsqrtmul(const bool m1pos, const bool m2pos, const Scalar asm1,
                        const Scalar asm2) {
    // asm1 = sqrt|m1|
    // asm2 = sqrt|m2|
    // <-- real(sqrt(m1) * sqrt(m2))

    // Real part is  asm1*asm2 if both pos
    //              -asm1*asm2 if both neg
    //              0          otherwise

    if (m1pos != m2pos) return 0;
    return (m1pos - !m1pos) * (asm1 * asm2);
}

void SVFBiquad::update(const Scalar b0, const Scalar b1, const Scalar b2, const Scalar a1,
                       const Scalar a2) {
    using namespace std::complex_literals;

    const Scalar m1 = -1 - a1 - a2;
    const Scalar m2 = -1 + a1 - a2;

    const bool   m1pos = (m1 > 0);
    const bool   m2pos = (m2 > 0);
    const Scalar asm1 = std::sqrt(std::abs(m1));
    const Scalar asm2 = std::sqrt(std::abs(m2));

    // sm1div2 = sqrt(m1) / sqrt(m2)
    // sm1mul2 = sqrt(m1) * sqrt(m2)
    //  working out all the cases by hand is faster
    //  because we're dealing with either pure real or pure imaginary numbers
    //  so complex operations are overkill

    const Scalar sm1div2 = rcsqrtdiv(m1pos, m2pos, asm1, asm2);
    const Scalar sm1mul2 = rcsqrtmul(m1pos, m2pos, asm1, asm2);

    _g = sm1div2;
    _R = (a2 - 1) / sm1mul2;
    _cHP = (b0 - b1 + b2) / (1 - a1 + a2);
    _cBP = (2 * (b0 - b2)) / sm1mul2;
    _cLP = (b0 + b1 + b2) / (1 + a1 + a2);

    _HP.update(_g, _R, SVFPiece::FltType_HIGHPASS);
    _BP.update(_g, _R, SVFPiece::FltType_BANDPASS);
    _LP.update(_g, _R, SVFPiece::FltType_LOWPASS);
}

Scalar SVFBiquad::tick(const Scalar x) {
    return _cHP * _HP.tick(x) + _cBP * _BP.tick(x) + _cLP * _LP.tick(x);
}