#ifndef SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H
#define SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H

#include <sigslot/signal.hpp>

#include "ScalarParameter.h"

struct GlottalFlowParameters {
    ScalarParameter Oq{"Oq", 0.45, 0.35, 1.0};
    ScalarParameter am{"am", 0.87, 0.6, 0.9};
    ScalarParameter Qa{"Qa", 0.09, 0, 0.5};

    // Using Rd parameter (only for LF model)
    bool usingRd() const { return _usingRd; }
    void setUsingRd(bool flag) {
        if (_usingRd != flag) {
            _usingRd = flag;
            usingRdChanged(flag);
        }
    }
    sigslot::signal<bool> usingRdChanged;

    ScalarParameter Rd{"Rd", 0.32, 0.01, 6.00};

   private:
    bool _usingRd{true};
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_PARAMETERS_H
