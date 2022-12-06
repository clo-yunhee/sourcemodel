#ifndef SOURCEMODEL__GLOTTAL_FLOW_MODEL_H
#define SOURCEMODEL__GLOTTAL_FLOW_MODEL_H

#include "GlottalFlowParameters.h"

class GlottalFlowModel {
   public:
    virtual ~GlottalFlowModel() {}

    virtual double evaluate(double t) = 0;

    virtual bool fitParameters(const GlottalFlowParameters& params) = 0;
    virtual void updateParameterBounds(GlottalFlowParameters& params) = 0;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_MODEL_H