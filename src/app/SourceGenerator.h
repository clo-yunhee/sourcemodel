#ifndef SOURCEMODEL__SOURCE_GENERATOR_H
#define SOURCEMODEL__SOURCE_GENERATOR_H

#include <NFParam/Param.h>

#include <atomic>

#include "GlottalFlowModel.h"
#include "GlottalFlowParameters.h"
#include "audio/BufferedGenerator.h"
#include "math/filters/Butterworth.h"
#include "math/filters/SOSFilter.h"

class GlottalFlow;

class SourceGenerator : public BufferedGenerator {
   public:
    SourceGenerator(const AudioTime& time, GlottalFlow& glottalFlow);

    double pitch() const;
    void   setPitch(double f0);

    void setSampleRate(double fs);

    void handleModelChanged(GlottalFlowModelType type);
    void handleParamChanged(const std::string& name, double value);
    void handleUsingRdChanged(bool usingRd);

   protected:
    void fillInternalBuffer(std::vector<double>& out) override;

   private:
    void handleInternalParamChanged(const std::string&, double);

    GlottalFlow& m_glottalFlow;

    // GFM instance to handle bound checking and such.
    GlottalFlowParameters m_gfmParameters;

    // NFParam for each parameter.
    std::shared_ptr<nativeformat::param::Param> m_Oq;
    std::shared_ptr<nativeformat::param::Param> m_am;
    std::shared_ptr<nativeformat::param::Param> m_Qa;
    std::shared_ptr<nativeformat::param::Param> m_Rd;
    std::shared_ptr<nativeformat::param::Param> m_f0;

    double m_targetF0;
    double m_fs;

    double           m_gfmTime;
    std::atomic_bool m_internalParamChanged;

    bool        m_needsToRecreateFilters;
    Butterworth m_antialiasFilter;
};

#endif  // SOURCEMODEL__SOURCE_GENERATOR_H