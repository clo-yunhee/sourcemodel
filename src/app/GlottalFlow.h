#ifndef SOURCEMODEL__GLOTTAL_FLOW_H
#define SOURCEMODEL__GLOTTAL_FLOW_H

#include <memory>

#include "GlottalFlowModel.h"
#include "GlottalFlowParameters.h"
#include "integration/NewtonCotes.h"

class GlottalFlow {
   public:
    GlottalFlow();

    GlottalFlowParameters& parameters();

    template <typename T>
    void setModel() {
        m_model = std::make_unique<T>();
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        connectSignalsToModel();
        markDirty();
    }

    void markDirty();
    bool isDirty() const;

    void updateSamples();

    void setSampleCount(int sampleCount);
    int  sampleCount() const;

    const double* t() const;
    const double* dg() const;
    const double* g() const;

   private:
    void connectSignalsToModel();

    GlottalFlowParameters m_parameters;
    NewtonCotes           m_integrator;

    std::unique_ptr<GlottalFlowModel> m_model;

    bool                m_isDirty;
    int                 m_sampleCount;
    std::vector<double> m_times;
    std::vector<double> m_flowDerivative;
    std::vector<double> m_flow;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_H
