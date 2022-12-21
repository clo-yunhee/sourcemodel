#include "SourceGenerator.h"

#include "GlottalFlow.h"
#include "math/filters/AAF.h"

using nativeformat::param::createParam;

SourceGenerator::SourceGenerator(const AudioTime& time, GlottalFlow& glottalFlow)
    : BufferedGenerator(time),
      m_glottalFlow(glottalFlow),
      m_gfmTime(0),
      m_internalParamChanged(true),
      m_needsToRecreateFilters(true) {
    setPitch(200);
    setSampleRate(48000);

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

double SourceGenerator::pitch() const { return m_targetF0; }

void SourceGenerator::setPitch(const double f0) {
    if (m_f0) {
        m_f0->linearRampToValueAtTime(f0, time() + 0.1);
    } else {
        m_f0 = createParam(f0, 1000.0, 16.0, "f0");
    }
    m_targetF0 = f0;
}

void SourceGenerator::setSampleRate(const double fs) {
    m_fs = fs;
    // Async filter creation
    m_needsToRecreateFilters = true;
}

void SourceGenerator::handleModelChanged(const GlottalFlowModelType type) {
    m_internalParamChanged = true;
}

void SourceGenerator::handleParamChanged(const std::string& name, const double value) {
    if (name == "Rd") {
        if (m_Rd) {
            m_Rd->linearRampToValueAtTime(value, time() + 0.1);
        } else {
            m_Rd = createParam(value, 6.00, 0.01, name);
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
        (*param)->linearRampToValueAtTime(value, time() + 0.1);
    } else {
        *param = createParam(value, 1.0, 0.0, name);
    }

    m_internalParamChanged = true;
}

void SourceGenerator::handleUsingRdChanged(const bool usingRd) {
    m_gfmParameters.setUsingRd(usingRd);
    m_internalParamChanged = true;
}

void SourceGenerator::fillInternalBuffer(std::vector<double>& out) {
    auto model = m_glottalFlow.genModel().lock();
    if (!model) {
        // genModel expired
        return;
    }

    for (int i = 0; i < out.size(); ++i) {
        const double t = time(i);

        // Rebuild model if needed.
        if (m_internalParamChanged) {
            m_internalParamChanged = false;
            model->updateParameterBounds(m_gfmParameters);
            model->fitParameters(m_gfmParameters);
        }

        // Evaluate.
        out[i] = 0.125 * model->evaluate(m_gfmTime);

        m_gfmTime += m_f0->valueForTime(t) / m_fs;

        if (m_gfmTime >= 1.0) {
            // Update parameters every period.
            if (m_Oq) m_gfmParameters.Oq.setValue(m_Oq->valueForTime(t));
            if (m_am) m_gfmParameters.am.setValue(m_am->valueForTime(t));
            if (m_Qa) m_gfmParameters.Qa.setValue(m_Qa->valueForTime(t));
            if (m_Rd) m_gfmParameters.Rd.setValue(m_Rd->valueForTime(t));
            m_gfmTime = fmod(m_gfmTime, 1.0);
        }
    }

    // Async filter creation
    if (m_needsToRecreateFilters) {
        m_antialiasFilter.loPass(m_fs, m_fs * 0.47, 5);
        m_needsToRecreateFilters = false;
    }

    out = m_antialiasFilter.filter(out);

    // Prune past parameter events
    if (m_Oq) m_Oq->pruneEventsPriorToTime(time());
    if (m_am) m_am->pruneEventsPriorToTime(time());
    if (m_Qa) m_Qa->pruneEventsPriorToTime(time());
    if (m_Rd) m_Rd->pruneEventsPriorToTime(time());
    if (m_f0) m_f0->pruneEventsPriorToTime(time());
}

void SourceGenerator::handleInternalParamChanged(const std::string&, double) {
    m_internalParamChanged = true;
}
