#ifndef SOURCEMODEL__MODELS_LF_H
#define SOURCEMODEL__MODELS_LF_H

#include "../GlottalFlowModel.h"

namespace models {
class LF : public GlottalFlowModel {
   public:
    ~LF() override {}

    double evaluate(double t) const override;

    void fitParameters(const GlottalFlowParameters& params) override;
    void updateParameterBounds(GlottalFlowParameters& params) override;

    // Specific to LF model, get Te directly, used for when Rd is used.
    double Te() const;

   private:
    void fitParameters(double Ee, double T0, double Te, double Tp, double Ta);

    double m_Ee;  // calculated for E0 = 1
    double m_Te;  // = Oq * T0
    double m_Tp;  // = am * Oq * T0
    double m_Ta;  // = Qa * (1 - Oq) * T0

    double m_epsilon;
    double m_alpha;

    // Pre-computed Rd lookup table.
    static double Rd_min;
    static double Rd_max;
    static double Rd_step;
    // Tuple to be unpacked.
    static std::vector<std::tuple<double, double, double, double, double>> Rd_table;
};
}  // namespace models

#endif  //  SOURCEMODEL__MODELS_LF_H