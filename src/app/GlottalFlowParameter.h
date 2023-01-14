#ifndef SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
#define SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H

#include <cfloat>
#include <sigslot/signal.hpp>
#include <string>

#include "math/utils.h"

class GlottalFlowParameter {
   public:
    GlottalFlowParameter(const std::string& name, Scalar initial, Scalar min, Scalar max);

    const std::string& name() const;

    Scalar                                      value() const;
    void                                        setValue(Scalar);
    sigslot::signal<const std::string&, Scalar> valueChanged;

    Scalar min() const;
    void   setMin(Scalar);

    Scalar max() const;
    void   setMax(Scalar);

    bool isFixed() const;
    void setFixed(Scalar);

   private:
    void enforceBounds();

    std::string m_name;
    Scalar      m_value;
    Scalar      m_min;
    Scalar      m_max;

    bool m_isFixed;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
