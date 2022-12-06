#include "GlottalFlow.h"

GlottalFlow::GlottalFlow()
    : m_integrator(3, NewtonCotes::CLOSED, 1e-7, 5), m_isDirty(false), m_sampleCount(0) {}

GlottalFlowParameters& GlottalFlow::parameters() { return m_parameters; }

void GlottalFlow::markDirty() { m_isDirty = true; }

bool GlottalFlow::isDirty() const { return m_isDirty; }

void GlottalFlow::updateSamples() {
    m_times.resize(m_sampleCount);
    m_flowDerivative.resize(m_sampleCount);
    m_flow.resize(m_sampleCount);

    for (int i = 0; i < m_sampleCount; ++i) {
        m_times[i] = i / double(m_sampleCount);
        m_flowDerivative[i] = m_model->evaluate(m_times[i]);
        m_flow[i] = m_integrator.integrate(
            [this](double x) { return m_model->evaluate(x); }, 0, m_times[i]);
    }

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

void GlottalFlow::connectSignalsToModel() {
    m_parameters.Oq.valueChanged.connect([&](double) {
        m_model->fitParameters(m_parameters);
        markDirty();
    });
    m_parameters.am.valueChanged.connect([&](double) {
        m_model->fitParameters(m_parameters);
        markDirty();
    });
    m_parameters.Qa.valueChanged.connect([&](double) {
        m_model->fitParameters(m_parameters);
        markDirty();
    });
}