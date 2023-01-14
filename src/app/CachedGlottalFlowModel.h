#ifndef SOURCEMODEL__CACHED_GLOTTAL_FLOW_MODEL_H
#define SOURCEMODEL__CACHED_GLOTTAL_FLOW_MODEL_H

#include "GlottalFlowModel.h"

class CachedGlottalFlowModel {
   public:
    CachedGlottalFlowModel();

    Scalar get(int i);

    void setPeriodLength(int periodLength);
    void markModelChanged();

    bool isDirty() const;

    void updateCache(GlottalFlowModel* model);

   private:
    int                 m_periodLength;
    std::vector<Scalar> m_cachedValues;
    bool                m_isDirty;
};

#endif  //  SOURCEMODEL__CACHED_GLOTTAL_FLOW_MODEL_H