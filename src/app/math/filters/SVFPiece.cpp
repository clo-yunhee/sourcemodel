#include "SVFPiece.h"

SVFPiece::SVFPiece() { _z1 = _z2 = 0.0; }

void SVFPiece::update(const double g, const double R, const FltType type) {
    _type = type;
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
        default:
            return LP;
    }
}