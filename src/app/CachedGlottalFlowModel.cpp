#include "CachedGlottalFlowModel.h"

#include <iostream>

CachedGlottalFlowModel::CachedGlottalFlowModel() : m_periodLength(0), m_isDirty(true) {}

Scalar CachedGlottalFlowModel::get(const int i) {
    if (i < 0 || i >= m_periodLength) {
        std::cerr << "Missing GFM cache value" << std::endl;
        return 0;  // silently cache miss
    }
    return m_cachedValues[i];
}

void CachedGlottalFlowModel::setPeriodLength(const int periodLength) {
    if (m_periodLength != periodLength) {
        m_periodLength = periodLength;
        m_isDirty = true;
    }
}

void CachedGlottalFlowModel::markModelChanged() { m_isDirty = true; }

bool CachedGlottalFlowModel::isDirty() const { return m_isDirty; }

void CachedGlottalFlowModel::updateCache(GlottalFlowModel* model) {
    m_cachedValues.resize(m_periodLength);
    for (int i = 0; i < m_periodLength; ++i) {
        // Time-reverse it.
        m_cachedValues[i] =
            model->evaluate(Scalar(m_periodLength - 1 - i) / Scalar(m_periodLength - 1));
    }
    m_isDirty = false;
}