#ifndef SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
#define SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H

#include <cfloat>
#include <sigslot/signal.hpp>
#include <string>

class GlottalFlowParameter {
   public:
    GlottalFlowParameter(const std::string& name, double initial, double min, double max);

    const std::string& name() const;

    double                                      value() const;
    void                                        setValue(double);
    sigslot::signal<const std::string&, double> valueChanged;

    double min() const;
    void   setMin(double);

    double max() const;
    void   setMax(double);

    bool isFixed() const;
    void setFixed(double);

   private:
    void enforceBounds();

    std::string m_name;
    double      m_value;
    double      m_min;
    double      m_max;

    bool m_isFixed;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
