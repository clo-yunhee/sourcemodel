#ifndef SOURCEMODEL__GLOTTAL_FLOW_MODEL_H
#define SOURCEMODEL__GLOTTAL_FLOW_MODEL_H

#include "GlottalFlowParameters.h"

class GlottalFlowModel {
   public:
    virtual ~GlottalFlowModel() {}

    virtual bool hasAntiderivative() const { return false; }

    virtual Scalar evaluate(Scalar t) const = 0;
    virtual Scalar evaluateAntiderivative(Scalar t) const { return 0; }

    virtual void fitParameters(GlottalFlowParameters& params) = 0;
    virtual void updateParameterBounds(GlottalFlowParameters& params) = 0;
};

enum GlottalFlowModelType {
    GlottalFlowModel_LF = 0,
    GlottalFlowModel_RPlusPlus,
    GlottalFlowModel_RosenbergC,
    GlottalFlowModel_KLGLOTT88,
};

inline constexpr const char* GlottalFlowModel_NAMES = "LF\0R++\0Rosenberg-C\0KLGLOTT88\0";

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_MODEL_H