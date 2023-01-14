#ifndef SOURCEMODEL__GLOTTAL_FLOW_H
#define SOURCEMODEL__GLOTTAL_FLOW_H

#include <memory>
#include <sigslot/signal.hpp>

#include "GlottalFlowModel.h"
#include "GlottalFlowParameters.h"

class GlottalFlow {
   public:
    GlottalFlow();

    GlottalFlowParameters& parameters();

    GlottalFlowModelType                  modelType() const;
    void                                  setModelType(GlottalFlowModelType);
    sigslot::signal<GlottalFlowModelType> modelTypeChanged;

    void markDirty();
    bool isDirty() const;

    void updateSamples();

    void setSampleCount(int sampleCount);
    int  sampleCount() const;

    const Scalar* t() const;
    const Scalar* dg() const;
    const Scalar* g() const;

    const std::pair<Scalar, Scalar>& dgMin() const;
    const std::pair<Scalar, Scalar>& dgMax() const;
    Scalar                           dgAmplitude() const;

    const std::pair<Scalar, Scalar>& gMin() const;
    const std::pair<Scalar, Scalar>& gMax() const;
    Scalar                           gAmplitude() const;

    std::weak_ptr<GlottalFlowModel>       genModel();
    std::weak_ptr<const GlottalFlowModel> genModel() const;

   private:
    template <typename T>
    void setModel() {
        m_model = std::make_unique<T>();
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        markDirty();
        // Another GFM instance used for generating audio.
        // Don't init parameters because that'll be done by the generator.
        m_modelForGen = std::make_unique<T>();
    }

    void paramChanged(const std::string& name, Scalar value);
    void usingRdChanged(bool usingRd);

    GlottalFlowParameters m_parameters;

    GlottalFlowModelType              m_modelType;
    std::unique_ptr<GlottalFlowModel> m_model;

    // Another GFM instance used for generating audio.
    std::shared_ptr<GlottalFlowModel> m_modelForGen;

    bool m_isDirty;

    int                       m_sampleCount;
    std::vector<Scalar>       m_times;
    std::vector<Scalar>       m_flowDerivative;
    Scalar                    m_flowDerivativeAmplitude;
    std::pair<Scalar, Scalar> m_flowDerivativeMin;
    std::pair<Scalar, Scalar> m_flowDerivativeMax;
    std::vector<Scalar>       m_flow;
    Scalar                    m_flowAmplitude;
    std::pair<Scalar, Scalar> m_flowMin;
    std::pair<Scalar, Scalar> m_flowMax;
};

#endif  //  SOURCEMODEL__GLOTTAL_FLOW_H
