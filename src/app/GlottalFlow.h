#ifndef SOURCEMODEL__GLOTTAL_FLOW_H
#define SOURCEMODEL__GLOTTAL_FLOW_H

#include <memory>

#include "GlottalFlowModel.h"
#include "GlottalFlowParameters.h"

class GlottalFlow {
   public:
    GlottalFlow();

    GlottalFlowParameters& parameters();

    void                 setModelType(GlottalFlowModelType);
    GlottalFlowModelType modelType() const;

    void markDirty();
    bool isDirty() const;

    void updateSamples();

    void setSampleCount(int sampleCount);
    int  sampleCount() const;

    const double* t() const;
    const double* dg() const;
    const double* g() const;

    const std::pair<double, double>& dgMin() const;
    const std::pair<double, double>& dgMax() const;
    double                           dgAmplitude() const;

    const std::pair<double, double>& gMin() const;
    const std::pair<double, double>& gMax() const;
    double                           gAmplitude() const;

   private:
    template <typename T>
    void setModel() {
        m_model = std::make_unique<T>();
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        markDirty();
    }

    GlottalFlowParameters m_parameters;

    GlottalFlowModelType              m_modelType;
    std::unique_ptr<GlottalFlowModel> m_model;

    bool                      m_isDirty;
    int                       m_sampleCount;
    std::vector<double>       m_times;
    std::vector<double>       m_flowDerivative;
    double                    m_flowDerivativeAmplitude;
    std::pair<double, double> m_flowDerivativeMin;
    std::pair<double, double> m_flowDerivativeMax;
    std::vector<double>       m_flow;
    double                    m_flowAmplitude;
    std::pair<double, double> m_flowMin;
    std::pair<double, double> m_flowMax;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_H
