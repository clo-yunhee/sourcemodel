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
    m_parameters.Oq.valueChanged.connect([&](double) {
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        markDirty();
    });

    m_parameters.am.valueChanged.connect([&](double) {
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        markDirty();
    });

    m_parameters.Qa.valueChanged.connect([&](double) {
        m_model->updateParameterBounds(m_parameters);
        m_model->fitParameters(m_parameters);
        markDirty();
    });
}

GlottalFlowParameters& GlottalFlow::parameters() { return m_parameters; }

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
}

GlottalFlowModelType GlottalFlow::modelType() const { return m_modelType; }

void GlottalFlow::markDirty() { m_isDirty = true; }

bool GlottalFlow::isDirty() const { return m_isDirty; }

void GlottalFlow::updateSamples() {
    m_times.resize(m_sampleCount);
    m_flowDerivative.resize(m_sampleCount);
    m_flow.resize(m_sampleCount);

    m_flowDerivativeMin.second = std::numeric_limits<double>::max();
    m_flowDerivativeMax.second = std::numeric_limits<double>::lowest();
    m_flowMin.second = 0;
    m_flowMax.second = 0;

    for (int i = 0; i < m_sampleCount; ++i) {
        m_times[i] = i / double(m_sampleCount);

        m_flowDerivative[i] = m_model->evaluate(m_times[i]);

        m_flow[i] = gauss_kronrod<double, 15>::integrate(
            [this](double t) { return m_model->evaluate(t); }, 0, m_times[i], 14, 1e-5);

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

void GlottalFlow::setSampleCount(int sampleCount) {
    if (m_sampleCount != sampleCount) {
        m_sampleCount = sampleCount;
        markDirty();
    }
}

int GlottalFlow::sampleCount() const { return m_sampleCount; }

const double* GlottalFlow::t() const { return m_times.data(); }

const double* GlottalFlow::dg() const { return m_flowDerivative.data(); }

const double* GlottalFlow::g() const { return m_flow.data(); }

const std::pair<double, double>& GlottalFlow::dgMin() const {
    return m_flowDerivativeMin;
}

const std::pair<double, double>& GlottalFlow::dgMax() const {
    return m_flowDerivativeMax;
}

double GlottalFlow::dgAmplitude() const { return m_flowDerivativeAmplitude; }

const std::pair<double, double>& GlottalFlow::gMin() const { return m_flowMin; }

const std::pair<double, double>& GlottalFlow::gMax() const { return m_flowMax; }

double GlottalFlow::gAmplitude() const { return m_flowAmplitude; }
