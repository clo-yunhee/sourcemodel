#ifndef SVF_PIECE_H
#define SVF_PIECE_H

#include <complex>

#include "math/utils.h"

/* Discrete-time generic SVF realized with TDF-II */

class SVFPiece {
   public:
    enum FltType {
        FltType_HIGHPASS,
        FltType_BANDPASS,
        FltType_LOWPASS,
    };

    SVFPiece();

    void update(Scalar g, Scalar R, FltType type);

    Scalar tick(Scalar x);

   private:
    FltType _type;

    Scalar _g;
    Scalar _R;
    Scalar _z1;
    Scalar _z2;
};

#endif  // SVF_PIECE_H