#include "GlottalFlow.h"

#include <boost/math/quadrature/gauss_kronrod.hpp>
#include <cmath>

#include "models/KLGLOTT88.h"
#include "models/LF.h"
#include "models/RPlusPlus.h"
#include "models/RosenbergC.h"

using namespace boost::math::quadrature;

GlottalFlow::GlottalFlow()
    : m_isDirty(false),
      m_sampleCount(0),
      m_flowDerivativeAmplitude(0),
      m_flowAmplitude(0) {
    m_parameters.Oq.valueChanged.connect(&GlottalFlow::paramChanged, this);
    m_parameters.am.valueChanged.connect(&GlottalFlow::paramChanged, this);
    m_parameters.Qa.valueChanged.connect(&GlottalFlow::paramChanged, this);
    m_parameters.usingRdChanged.connect(&GlottalFlow::usingRdChanged, this);
    m_parameters.Rd.valueChanged.connect(&GlottalFlow::paramChanged, this);
}

GlottalFlowParameters& GlottalFlow::parameters() { return m_parameters; }

GlottalFlowModelType GlottalFlow::modelType() const { return m_modelType; }

void GlottalFlow::setModelType(const GlottalFlowModelType modelType) {
    switch (modelType) {
        case GlottalFlowModel_LF:
            setModel<models::LF>();
            break;
        case GlottalFlowModel_RPlusPlus:
            setModel<models::RPlusPlus>();
            break;
        case GlottalFlowModel_RosenbergC:
            setModel<models::RosenbergC>();
            break;
        case GlottalFlowModel_KLGLOTT88:
            setModel<models::KLGLOTT88>();
            break;
    }
    m_modelType = modelType;
    modelTypeChanged(modelType);
}

void GlottalFlow::markDirty() { m_isDirty = true; }

bool GlottalFlow::isDirty() const { return m_isDirty; }

void GlottalFlow::updateSamples() {
    m_times.resize(m_sampleCount);
    m_flowDerivative.resize(m_sampleCount);
    m_flow.resize(m_sampleCount);

    m_flowDerivativeMin.second = std::numeric_limits<Scalar>::max();
    m_flowDerivativeMax.second = std::numeric_limits<Scalar>::lowest();
    m_flowMin.second = 0;
    m_flowMax.second = 0;

    Scalar Te;
    if (m_modelType == GlottalFlowModel_LF && m_parameters.usingRd()) {
        Te = ((const models::LF*)m_model.get())->Te();
    } else {
        Te = m_parameters.Oq.value();
    }

    for (int i = 0; i < m_sampleCount; ++i) {
        m_times[i] = i / Scalar(m_sampleCount);

        // Make sure to include Te to include the real peak in the plot.
        if (i < m_sampleCount - 1 && m_times[i] < Te && Te < m_times[i + 1]) {
            m_times[i] = Te;
        }

        m_flowDerivative[i] = m_model->evaluate(m_times[i]);

        if (m_model->hasAntiderivative()) {
            m_flow[i] = m_model->evaluateAntiderivative(m_times[i]);
        } else {
// Do faster integration on Emscripten
#ifdef __EMSCRIPTEN__
            m_flow[i] = gauss_kronrod<float, 15>::integrate(
                [this](float t) -> float { return m_model->evaluate(t); }, 0, m_times[i],
                15, 1e-3);
#else
    // Float / double
    #if defined(USING_SINGLE_FLOAT)
            m_flow[i] = gauss_kronrod<float, 15>::integrate(
                [this](double t) -> double { return m_model->evaluate(t); }, 0,
                m_times[i], 8, 1e-6);
    #elif defined(USING_DOUBLE_FLOAT) m_flow[i] =
            gauss_kronrod<double, 31>::integrate(
                [this](double t) -> double { return m_model->evaluate(t); }, 0,
                m_times[i], 15, 1e-6);
    #endif
#endif
        }

        if (m_flowDerivative[i] < m_flowDerivativeMin.second) {
            m_flowDerivativeMin.second = m_flowDerivative[i];
            m_flowDerivativeMin.first = m_times[i];
        }
        if (m_flowDerivative[i] > m_flowDerivativeMax.second) {
            m_flowDerivativeMax.second = m_flowDerivative[i];
            m_flowDerivativeMax.first = m_times[i];
        }

        if (m_flow[i] < m_flowMin.second) {
            m_flowMin.second = m_flow[i];
            m_flowMin.first = m_times[i];
        }
        if (m_flow[i] > m_flowMax.second) {
            m_flowMax.second = m_flow[i];
            m_flowMax.first = m_times[i];
        }
    }

    m_flowDerivativeAmplitude = std::max(std::abs(m_flowDerivativeMin.second),
                                         std::abs(m_flowDerivativeMax.second));
    m_flowAmplitude = std::max(std::abs(m_flowMin.second), std::abs(m_flowMax.second));

    m_isDirty = false;
}

void GlottalFlow::setSampleCount(const int sampleCount) {
    if (m_sampleCount != sampleCount) {
        m_sampleCount = sampleCount;
        markDirty();
    }
}

int GlottalFlow::sampleCount() const { return m_sampleCount; }

const Scalar* GlottalFlow::t() const { return m_times.data(); }

const Scalar* GlottalFlow::dg() const { return m_flowDerivative.data(); }

const Scalar* GlottalFlow::g() const { return m_flow.data(); }

const std::pair<Scalar, Scalar>& GlottalFlow::dgMin() const {
    return m_flowDerivativeMin;
}

const std::pair<Scalar, Scalar>& GlottalFlow::dgMax() const {
    return m_flowDerivativeMax;
}

Scalar GlottalFlow::dgAmplitude() const { return m_flowDerivativeAmplitude; }

const std::pair<Scalar, Scalar>& GlottalFlow::gMin() const { return m_flowMin; }

const std::pair<Scalar, Scalar>& GlottalFlow::gMax() const { return m_flowMax; }

Scalar GlottalFlow::gAmplitude() const { return m_flowAmplitude; }

std::weak_ptr<GlottalFlowModel> GlottalFlow::genModel() { return m_modelForGen; }

std::weak_ptr<const GlottalFlowModel> GlottalFlow::genModel() const {
    return m_modelForGen;
}

void GlottalFlow::paramChanged(const std::string& name, const Scalar value) {
    m_model->updateParameterBounds(m_parameters);
    m_model->fitParameters(m_parameters);
    markDirty();
}

void GlottalFlow::usingRdChanged(bool usingRd) {
    m_model->updateParameterBounds(m_parameters);
    m_model->fitParameters(m_parameters);
    markDirty();
}