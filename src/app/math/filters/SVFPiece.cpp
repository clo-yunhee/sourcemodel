#include "SVFPiece.h"

SVFPiece::SVFPiece() { _z1 = _z2 = 0.0_f; }

void SVFPiece::update(const Scalar g, const Scalar R, const FltType type) {
    _type = type;
    _g = g;
    _R = R;
}

Scalar SVFPiece::tick(const Scalar x) {
    const Scalar HP =
        (x - (2.0_f * _R + _g) * _z1 - _z2) / (1.0_f + (2.0_f * _R * _g) + _g * _g);
    const Scalar BP = HP * _g + _z1;
    const Scalar LP = BP * _g + _z2;

    _z1 = _g * HP + BP;
    _z2 = _g * BP + LP;

    switch (_type) {
        case FltType_HIGHPASS:
            return HP;
        case FltType_BANDPASS:
            return BP;
        case FltType_LOWPASS:
        default:
            return LP;
    }
}