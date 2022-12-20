#ifndef SOURCEMODEL__MODELS_RPLUSPLUS_H
#define SOURCEMODEL__MODELS_RPLUSPLUS_H

#include "../GlottalFlowModel.h"

namespace models {
class RPlusPlus : public GlottalFlowModel {
   public:
    ~RPlusPlus() override {}

    double evaluate(double t) const override;

    bool fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

   private:
    double m_K;
    double m_Te;
    double m_Tp;
    double m_Ta;

    double m_Tx;

    double m_dgTe;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_RPLUSPLUS_H