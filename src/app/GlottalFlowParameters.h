#ifndef SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H
#define SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H

#include "GlottalFlowParameter.h"

struct GlottalFlowParameters {
    GlottalFlowParameter Oq{"Oq", 0.45, 0.35, 1.0};
    GlottalFlowParameter am{"am", 0.87, 0.6, 0.9};
    GlottalFlowParameter Qa{"Qa", 0.09, 0, 0.5};
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H
