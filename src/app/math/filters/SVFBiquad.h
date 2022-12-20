#ifndef SVF_BIQUAD_H
#define SVF_BIQUAD_H

#include "SVFPiece.h"

/* Biquad realized with SVFs */

class SVFBiquad {
   public:
    void update(double b0, double b1, double b2, double a1, double a2);

    double tick(double x);

   private:
    double _g;
    double _R;
    double _cHP;
    double _cBP;
    double _cLP;

    SVFPiece _HP;
    SVFPiece _BP;
    SVFPiece _LP;
};

#endif  // SVF_BIQUAD_H