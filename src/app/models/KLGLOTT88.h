#ifndef SOURCEMODEL__MODELS_KLGLOTT88_H
#define SOURCEMODEL__MODELS_KLGLOTT88_H

#include "../GlottalFlowModel.h"

namespace models {
class KLGLOTT88 : public GlottalFlowModel {
   public:
    ~KLGLOTT88() override {}

    bool hasAntiderivative() const override { return true; }

    Scalar evaluate(Scalar t) const override;
    Scalar evaluateAntiderivative(Scalar t) const override;

    void fitParameters(GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    Scalar m_Oq;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_KLGLOTT88_H