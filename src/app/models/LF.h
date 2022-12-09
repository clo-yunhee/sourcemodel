#ifndef SOURCEMODEL__MODELS_LF_H
#define SOURCEMODEL__MODELS_LF_H

#include "../GlottalFlowModel.h"

namespace models {
class LF : public GlottalFlowModel {
   public:
    ~LF() override {}

    double evaluate(double t) override;

    bool fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    double m_Ee;  // calculated for E0 = 1
    double m_Te;  // = Oq * T0
    double m_Tp;  // = am * Oq * T0
    double m_Ta;  // = Qa * (1 - Oq) * T0

    double m_epsilon;
    double m_alpha;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_LF_H