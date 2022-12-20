#ifndef SOURCEMODEL__MODELS_ROSENBERG_C_H
#define SOURCEMODEL__MODELS_ROSENBERG_C_H

#include "../GlottalFlowModel.h"

namespace models {
class RosenbergC : public GlottalFlowModel {
   public:
    ~RosenbergC() override {}

    double evaluate(double t) const override;

    bool fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    double m_A;
    double m_Tp;
    double m_Tn;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_ROSENBERG_C_H