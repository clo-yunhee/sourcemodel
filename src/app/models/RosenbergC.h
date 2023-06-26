#ifndef SOURCEMODEL__MODELS_ROSENBERG_C_H
#define SOURCEMODEL__MODELS_ROSENBERG_C_H

#include "../GlottalFlowModel.h"

namespace models {
class RosenbergC : public GlottalFlowModel {
   public:
    ~RosenbergC() override {}

    bool hasAntiderivative() const override { return true; }

    Scalar evaluate(Scalar t) const override;
    Scalar evaluateAntiderivative(Scalar t) const override;

    void fitParameters(GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    Scalar m_A;
    Scalar m_Tp;
    Scalar m_Tn;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_ROSENBERG_C_H