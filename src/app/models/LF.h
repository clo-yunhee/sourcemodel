#ifndef SOURCEMODEL__MODELS_LF_H
#define SOURCEMODEL__MODELS_LF_H

#include <array>
#include <utility>

#include "../GlottalFlowModel.h"

namespace models {
class LF : public GlottalFlowModel {
   public:
    ~LF() override {}

    bool hasAntiderivative() const override { return true; }

    Scalar evaluate(Scalar t) const override;
    Scalar evaluateAntiderivative(Scalar t) const override;

    void fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

    // Specific to LF model, get Te directly, used for when Rd is used.
    Scalar Te() const;

   private:
    void fitParameters(Scalar Ee, Scalar T0, Scalar Te, Scalar Tp, Scalar Ta);

    Scalar m_Ee;  // calculated for E0 = 1
    Scalar m_Te;  // = Oq * T0
    Scalar m_Tp;  // = am * Oq * T0
    Scalar m_Ta;  // = Qa * (1 - Oq) * T0

    Scalar m_epsilon;
    Scalar m_alpha;

    Scalar m_gTe;
};

namespace precomp {
#if defined(USING_SINGLE_FLOAT)
    #include "LF_precomputed_Rd_float.inc.h"
#elif defined(USING_DOUBLE_FLOAT)
    #include "LF_precomputed_Rd_double.inc.h"
#endif
}  // namespace precomp
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_LF_H