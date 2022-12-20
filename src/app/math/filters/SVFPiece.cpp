#include "SVFPiece.h"

SVFPiece::SVFPiece() { _v0 = _v1 = _z1 = _z2 = 0.0; }

void SVFPiece::update(const double g, const double R, const FltType type) {
    double a0, a1, a2, b0, b1, b2;

    a0 = 1.0 + g * g + 2.0 * R * g;
    a1 = 2.0 * g * g - 2.0;
    a2 = 1.0 + g * g - 2.0 * R * g;

    switch (type) {
        case FltType_HIGHPASS:
            b0 = 1.0;
            b1 = -2.0;
            b2 = 1.0;
            break;
        case FltType_BANDPASS:
            b0 = g;
            b1 = 0.0;
            b2 = -g;
            break;
        case FltType_LOWPASS:
            b0 = g * g;
            b1 = 2.0 * g * g;
            b2 = g * g;
            break;
    }

    _type = type;
    _b0 = b0 / a0;
    _b1 = b1 / a0;
    _b2 = b2 / a0;
    _a1 = a1 / a0;
    _a2 = a2 / a0;

    _g = g;
    _R = R;
}

double SVFPiece::tick(const double x) {
    const double HP =
        (x - (2.0 * _R + _g) * _z1 - _z2) / (1.0 + (2.0 * _R * _g) + _g * _g);
    const double BP = HP * _g + _z1;
    const double LP = BP * _g + _z2;

    _z1 = _g * HP + BP;
    _z2 = _g * BP + LP;

    switch (_type) {
        case FltType_HIGHPASS:
            return HP;
        case FltType_BANDPASS:
            return BP;
        case FltType_LOWPASS:
            return LP;
    }
}