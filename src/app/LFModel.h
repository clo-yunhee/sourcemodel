#ifndef SOURCEMODEL__LF_MODEL_H
#define SOURCEMODEL__LF_MODEL_H

#include "GlottalFlowModel.h"

class LFModel : public GlottalFlowModel {
   public:
    ~LFModel() override {}

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

#endif  //  SOURCEMODEL__LF_MODEL_H