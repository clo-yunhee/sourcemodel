#include "SourceGenerator.h"

#include "GlottalFlow.h"

using nativeformat::param::createParam;

SourceGenerator::SourceGenerator(const AudioTime& time, GlottalFlow& glottalFlow)
    : BufferedGenerator(time),
      m_glottalFlow(glottalFlow),
      m_currentPeriod(0),
      m_internalParamChanged(true) {
    setPitch(120);

    m_gfmParameters.Oq.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.am.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.Qa.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
    m_gfmParameters.usingRdChanged.connect(&SourceGenerator::handleUsingRdChanged, this);
    m_gfmParameters.Rd.valueChanged.connect(&SourceGenerator::handleInternalParamChanged,
                                            this);
}

Scalar SourceGenerator::pitch() const { return m_targetF0; }

void SourceGenerator::setPitch(const Scalar f0) {
    if (m_f0) {
        m_f0->linearRampToValueAtTime(f0, time() + 0.1_f);
    } else {
        m_f0 = createParam(f0, 1000.0_f, 16.0_f, "f0");
    }
    m_targetF0 = f0;
}

void SourceGenerator::handleModelChanged(const GlottalFlowModelType type) {
    m_internalParamChanged = true;
}

void SourceGenerator::handleParamChanged(const std::string& name, const Scalar value) {
    if (name == "Rd") {
        if (m_Rd) {
            m_Rd->exponentialRampToValueAtTime(value, time() + 0.1_f);
        } else {
            m_Rd = createParam(value, 6.00_f, 0.01_f, name);
        }
        m_internalParamChanged = true;
        return;
    }

    std::shared_ptr<nativeformat::param::Param>* param;

    if (name == "Oq") {
        param = &m_Oq;
    } else if (name == "am") {
        param = &m_am;
    } else if (name == "Qa") {
        param = &m_Qa;
    }

    if (*param) {
        (*param)->linearRampToValueAtTime(value, time() + 0.1_f);
    } else {
        *param = createParam(value, 1.0_f, 0.0_f, name);
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
        out[i] = m_cachedGfm.get(m_currentTime);

        ++m_currentTime;

        if (m_currentTime >= m_currentPeriod) {
            // Update parameters every period.
            if (m_Oq) m_gfmParameters.Oq.setValue(m_Oq->valueForTime(t));
            if (m_am) m_gfmParameters.am.setValue(m_am->valueForTime(t));
            if (m_Qa) m_gfmParameters.Qa.setValue(m_Qa->valueForTime(t));
            if (m_Rd) m_gfmParameters.Rd.setValue(m_Rd->valueForTime(t));

            m_currentF0 = m_f0->valueForTime(t);
            m_currentPeriod = std::round(fs() / m_currentF0);
            m_currentTime = 0;
            m_cachedGfm.setPeriodLength(m_currentPeriod);

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
    if (m_Oq) m_Oq->pruneEventsPriorToTime(time());
    if (m_am) m_am->pruneEventsPriorToTime(time());
    if (m_Qa) m_Qa->pruneEventsPriorToTime(time());
    if (m_Rd) m_Rd->pruneEventsPriorToTime(time());
    if (m_f0) m_f0->pruneEventsPriorToTime(time());
}

void SourceGenerator::handleInternalParamChanged(const std::string&, Scalar) {
    m_internalParamChanged = true;
}
