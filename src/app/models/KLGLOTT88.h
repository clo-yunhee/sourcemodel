#ifndef SOURCEMODEL__MODELS_KLGLOTT88_H
#define SOURCEMODEL__MODELS_KLGLOTT88_H

#include "../GlottalFlowModel.h"

namespace models {
class KLGLOTT88 : public GlottalFlowModel {
   public:
    ~KLGLOTT88() override {}

    double evaluate(double t) override;

    bool fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    double m_Oq;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_KLGLOTT88_H