#ifndef SOURCEMODEL__MODELS_RPLUSPLUS_H
#define SOURCEMODEL__MODELS_RPLUSPLUS_H

#include "../GlottalFlowModel.h"

namespace models {
class RPlusPlus : public GlottalFlowModel {
   public:
    ~RPlusPlus() override {}

    bool hasAntiderivative() const override { return true; }

    Scalar evaluate(Scalar t) const override;
    Scalar evaluateAntiderivative(Scalar t) const override;

    void fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    Scalar m_K;
    Scalar m_Te;
    Scalar m_Tp;
    Scalar m_Ta;

    Scalar m_Tx;

    Scalar m_dgTe;
    Scalar m_gTe;

    Scalar m_expT0TeTa;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_RPLUSPLUS_H