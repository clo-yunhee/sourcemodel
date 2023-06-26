#ifndef SOURCEMODEL__SOURCE_GENERATOR_H
#define SOURCEMODEL__SOURCE_GENERATOR_H

#include <NFParam/Param.h>

#include <atomic>
#include <random>

#include "CachedGlottalFlowModel.h"
#include "GlottalFlowModel.h"
#include "GlottalFlowParameters.h"
#include "ToggleParameter.h"
#include "audio/BufferedGenerator.h"
#include "audio/LookAheadGainReduction.h"
#include "math/PinkNoise.h"
#include "math/filters/Butterworth.h"
#include "math/filters/SOSFilter.h"
#include "math/utils.h"

class GlottalFlow;

class SourceGenerator : public BufferedGenerator {
   public:
    SourceGenerator(const AudioTime& time, GlottalFlow& glottalFlow);

    ScalarParameter& pitch();
    ScalarParameter& flutter();
    ScalarParameter& jitter();
    ScalarParameter& shimmer();

    ToggleParameter& flutterToggle();
    ToggleParameter& jitterToggle();
    ToggleParameter& shimmerToggle();

    void handleModelChanged(GlottalFlowModelType type);
    void handleParamChanged(const std::string& name, Scalar value);
    void handleUsingRdChanged(bool usingRd);

   protected:
    void fillInternalBuffer(std::vector<Scalar>& out) override;

   private:
    void handleInternalParamChanged(const std::string&, Scalar);

    GlottalFlow& m_glottalFlow;

    // GFM instance to handle bound checking and such.
    GlottalFlowParameters  m_gfmParameters;
    CachedGlottalFlowModel m_cachedGfm;

    // NFParam for each parameter.
    std::shared_ptr<nativeformat::param::Param> m_Oq;
    std::shared_ptr<nativeformat::param::Param> m_am;
    std::shared_ptr<nativeformat::param::Param> m_Qa;
    std::shared_ptr<nativeformat::param::Param> m_Rd;
    std::shared_ptr<nativeformat::param::Param> m_f0;
    std::shared_ptr<nativeformat::param::Param> m_Fpmax;
    std::shared_ptr<nativeformat::param::Param> m_Jmax;
    std::shared_ptr<nativeformat::param::Param> m_Smax;

    ScalarParameter m_paramF0;
    ScalarParameter m_paramFlutter;
    ScalarParameter m_paramJitter;
    ScalarParameter m_paramShimmer;

    ToggleParameter m_paramFlutterToggle;
    ToggleParameter m_paramJitterToggle;
    ToggleParameter m_paramShimmerToggle;

    std::random_device              m_randomDevice;
    std::mt19937                    m_randomGenerator;
    PinkNoise                       m_flutterPinkNoise;
    std::discrete_distribution<int> m_jitterDistribution;

    Scalar m_currentF0;
    int    m_currentPeriod;
    int    m_currentTime;
    Scalar m_currentShimmer;

    Scalar           m_gfmTime;
    std::atomic_bool m_internalParamChanged;

    Butterworth m_antialiasFilter;
};

#endif  // SOURCEMODEL__SOURCE_GENERATOR_H