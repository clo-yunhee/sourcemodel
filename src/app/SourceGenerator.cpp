#include "SourceGenerator.h"

#include <boost/math/special_functions/sin_pi.hpp>

#include "GlottalFlow.h"

using boost::math::sin_pi;
using nativeformat::param::createParam;

namespace {
constexpr std::array<Scalar, 13> jitterDistributionWeights = {
    1 / 64., 2 / 64., 3 / 64., 5 / 64., 7 / 64., 9 / 64., 10 / 64.,
    9 / 64., 7 / 64., 5 / 64., 3 / 64., 2 / 64., 1 / 64.};

constexpr std::array<Scalar, 13> jitterDistributionDeviations = {
    -3, -1.9, -1.48, -1.12, -0.76, -0.38, 0, 0.38, 0.76, 1.12, 1.48, 1.9, 3};
}  // namespace

SourceGenerator::SourceGenerator(const AudioTime& time, GlottalFlow& glottalFlow)
    : BufferedGenerator(time),
      m_glottalFlow(glottalFlow),
      m_currentPeriod(0),
      m_internalParamChanged(true),
      m_paramF0("f0", 120, 16, 1000),
      m_paramFlutter("Fpmax", 0.02, 0, 0.5),
      m_paramJitter("Jmax", 0.005, 0, 0.2),
      m_paramShimmer("Smax", 0.015, 0, 0.2),
      m_paramFlutterToggle("Fpon", true),
      m_paramJitterToggle("Jon", true),
      m_paramShimmerToggle("Son", true),
      m_randomGenerator(m_randomDevice()),
      m_jitterDistribution(jitterDistributionWeights.begin(),
                           jitterDistributionWeights.end()) {
    m_paramF0.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramFlutter.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramJitter.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramShimmer.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramFlutterToggle.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramJitterToggle.valueChanged.connect(&SourceGenerator::handleParamChanged, this);
    m_paramShimmerToggle.valueChanged.connect(&SourceGenerator::handleParamChanged, this);

    m_gfmParameters.Oq.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.am.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.Qa.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.usingRdChanged.connect(&SourceGenerator::handleUsingRdChanged, this);
    m_gfmParameters.Rd.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);

    m_f0 = m_paramF0.createParamFrom();
    m_Fpmax = m_paramFlutter.createParamFrom();
    m_Jmax = m_paramJitter.createParamFrom();
    m_Smax = m_paramShimmer.createParamFrom();
    m_Rd = m_gfmParameters.Rd.createParamFrom();
    m_Oq = createParam(m_gfmParameters.Oq.value(), 1.0f, 0.0f, "Oq");
    m_am = createParam(m_gfmParameters.am.value(), 1.0f, 0.0f, "am");
    m_Qa = createParam(m_gfmParameters.Qa.value(), 1.0f, 0.0f, "Qa");
}

ScalarParameter& SourceGenerator::pitch() { return m_paramF0; }

ScalarParameter& SourceGenerator::flutter() { return m_paramFlutter; }

ScalarParameter& SourceGenerator::jitter() { return m_paramJitter; }

ScalarParameter& SourceGenerator::shimmer() { return m_paramShimmer; }

ToggleParameter& SourceGenerator::flutterToggle() { return m_paramFlutterToggle; }

ToggleParameter& SourceGenerator::jitterToggle() { return m_paramJitterToggle; }

ToggleParameter& SourceGenerator::shimmerToggle() { return m_paramShimmerToggle; }

void SourceGenerator::handleModelChanged(const GlottalFlowModelType type) {
    m_internalParamChanged = true;
}

void SourceGenerator::handleParamChanged(const std::string& name, const Scalar value) {
    if (name == "f0") {
        m_f0->linearRampToValueAtTime(value, time() + 0.1_f);
    } else if (name == "Fpmax") {
        m_Fpmax->linearRampToValueAtTime(value, time() + 0.1_f);
    } else if (name == "Fpon") {
        // If on => set Fpmax to current value of paramFlutter
        // If off => set Fpmax to 0
        m_Fpmax->linearRampToValueAtTime(value * m_paramFlutter.value(), time() + 0.1_f);
    } else if (name == "Jmax") {
        m_Jmax->linearRampToValueAtTime(value, time() + 0.1_f);
    } else if (name == "Jon") {
        // If on => set Jmax to current value of paramJitter
        // If off => set Jmax to 0
        m_Jmax->linearRampToValueAtTime(value * m_paramJitter.value(), time() + 0.1_f);
    } else if (name == "Smax") {
        m_Smax->linearRampToValueAtTime(value, time() + 0.1_f);
    } else if (name == "Son") {
        // If on => set Smax to current value of paramShimmer
        // If off => set Smax to 0
        m_Smax->linearRampToValueAtTime(value * m_paramShimmer.value(), time() + 0.1_f);
    } else if (name == "Rd") {
        m_Rd->exponentialRampToValueAtTime(value, time() + 0.1_f);
    } else {
        std::shared_ptr<nativeformat::param::Param>* param;

        if (name == "Oq") {
            param = &m_Oq;
        } else if (name == "am") {
            param = &m_am;
        } else if (name == "Qa") {
            param = &m_Qa;
        } else {
            return;
        }

        (*param)->linearRampToValueAtTime(value, time() + 0.1_f);
    }

    m_internalParamChanged = true;
}

void SourceGenerator::handleUsingRdChanged(const bool usingRd) {
    m_gfmParameters.setUsingRd(usingRd);
    m_internalParamChanged = true;
}

void SourceGenerator::fillInternalBuffer(std::vector<Scalar>& out) {
    auto model = m_glottalFlow.genModel().lock();
    if (!model) {
        // genModel expired
        return;
    }

    if (m_currentPeriod == 0) {
        m_currentF0 = m_f0->valueForTime(time());
        m_currentPeriod = std::round(fs() / m_currentF0);
        m_currentTime = 0;
        m_currentShimmer = 1;
        // Init model
        model->updateParameterBounds(m_gfmParameters);
        model->fitParameters(m_gfmParameters);
        m_cachedGfm.setPeriodLength(m_currentPeriod);
        m_cachedGfm.updateCache(model.get());
    }

    for (int i = 0; i < out.size(); ++i) {
        const Scalar t = time(i);

        // Evaluate.
        /*out[i] =
            model->evaluate(1.0 - Scalar(m_currentTime) / Scalar(m_currentPeriod - 1));*/
        out[i] = m_cachedGfm.get(m_currentTime) * m_currentShimmer;

        ++m_currentTime;

        if (m_currentTime >= m_currentPeriod) {
            // Update parameters every period.
            m_gfmParameters.Oq.setValue(m_Oq->valueForTime(t));
            m_gfmParameters.am.setValue(m_am->valueForTime(t));
            m_gfmParameters.Qa.setValue(m_Qa->valueForTime(t));
            m_gfmParameters.Rd.setValue(m_Rd->valueForTime(t));

            m_currentF0 = m_f0->valueForTime(t);

            // Introduce flutter. Same strategy as KLATT90
            const Scalar Flmax = m_Fpmax->valueForTime(t);
            const Scalar Fln =
                .1 * (sin_pi(2 * 12.7 * t) + sin_pi(2 * 7.1 * t) + sin_pi(2 * 4.7 * t));
            m_currentF0 *= (1 + Flmax * Fln);

            // Introduce jitter.
            const Scalar Jmax = m_Jmax->valueForTime(t);
            const Scalar Jn =
                jitterDistributionDeviations[m_jitterDistribution(m_randomGenerator)];
            m_currentF0 *= (1 + Jmax * Jn);

            m_currentPeriod = std::round(fs() / m_currentF0);
            m_currentTime = 0;
            m_cachedGfm.setPeriodLength(m_currentPeriod);

            // Introduce shimmer.
            const Scalar Smax = m_Smax->valueForTime(t);
            const Scalar Sn =
                jitterDistributionDeviations[m_jitterDistribution(m_randomGenerator)];
            m_currentShimmer = (1 + Smax * Sn);

            // Rebuild model if needed.
            if (m_internalParamChanged) {
                m_internalParamChanged = false;
                model->updateParameterBounds(m_gfmParameters);
                model->fitParameters(m_gfmParameters);
                m_cachedGfm.markModelChanged();
            }

            // Recache GFM values if needed.
            if (m_cachedGfm.isDirty()) {
                m_cachedGfm.updateCache(model.get());
            }
        }
    }

    // Async filter creation
    if (hasSampleRateChanged()) {
        m_antialiasFilter.loPass(fs(), fs() / 2 - 1000, 1);
        ackSampleRateChange();
    }

    out = m_antialiasFilter.filter(out);

    // Prune past parameter events
    m_Oq->pruneEventsPriorToTime(time());
    m_am->pruneEventsPriorToTime(time());
    m_Qa->pruneEventsPriorToTime(time());
    m_Rd->pruneEventsPriorToTime(time());
    m_f0->pruneEventsPriorToTime(time());
    m_Fpmax->pruneEventsPriorToTime(time());
    m_Jmax->pruneEventsPriorToTime(time());
    m_Smax->pruneEventsPriorToTime(time());
}

void SourceGenerator::handleInternalParamChanged(const std::string&, Scalar) {
    m_internalParamChanged = true;
}
