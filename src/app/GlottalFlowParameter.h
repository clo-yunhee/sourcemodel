#ifndef SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
#define SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H

#include <cfloat>
#include <sigslot/signal.hpp>

class GlottalFlowParameter {
   public:
    GlottalFlowParameter(double initial = 0, double min = 0, double max = 1);

    void disconnectAll();

    double                  value() const;
    void                    setValue(double);
    sigslot::signal<double> valueChanged;

    double                  min() const;
    void                    setMin(double);
    sigslot::signal<double> minChanged;

    double                  max() const;
    void                    setMax(double);
    sigslot::signal<double> maxChanged;

    bool isFixed() const;
    void setFixed(double);

   private:
    void enforceBounds();

    double m_value;
    double m_min;
    double m_max;

    bool m_isFixed;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_PARAMETER_H
