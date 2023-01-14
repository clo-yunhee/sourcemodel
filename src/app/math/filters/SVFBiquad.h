#ifndef SVF_BIQUAD_H
#define SVF_BIQUAD_H

#include "SVFPiece.h"

/* Biquad realized with SVFs */

class SVFBiquad {
   public:
    void update(Scalar b0, Scalar b1, Scalar b2, Scalar a1, Scalar a2);

    Scalar tick(Scalar x);

   private:
    Scalar _g;
    Scalar _R;
    Scalar _cHP;
    Scalar _cBP;
    Scalar _cLP;

    SVFPiece _HP;
    SVFPiece _BP;
    SVFPiece _LP;
};

#endif  // SVF_BIQUAD_H