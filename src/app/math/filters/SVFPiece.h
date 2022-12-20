#ifndef SVF_PIECE_H
#define SVF_PIECE_H

#include <complex>

using dcomplex = std::complex<double>;

/* Discrete-time generic SVF realized with TDF-II */

class SVFPiece {
   public:
    enum FltType {
        FltType_HIGHPASS,
        FltType_BANDPASS,
        FltType_LOWPASS,
    };

    SVFPiece();

    void update(double g, double R, FltType type);

    double tick(double x);

   private:
    FltType _type;

    double _b0;
    double _b1;
    double _b2;
    double _a1;
    double _a2;

    double _v0;
    double _v1;

    double _g;
    double _R;
    double _z1;
    double _z2;
};

#endif  // SVF_PIECE_H