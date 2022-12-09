#ifndef SOURCEMODEL__SOURCEMODELAPP_H
#define SOURCEMODEL__SOURCEMODELAPP_H

#include "Application.h"
#include "GlottalFlow.h"

class SourceModelApp : public Application {
   public:
    SourceModelApp(int initialWidth = 800, int initialHeight = 600);
    virtual ~SourceModelApp();

   protected:
    void setupThemeColors(ImGuiStyle& style) override;
    void setupThemeSizes(ImGuiStyle& style) override;

    void renderMain() override;
    void renderOther() override;

   private:
    void renderParameterControl(GlottalFlowParameter& param, const char* name,
                                const char* displayName, int precision);

    GlottalFlow m_glottalFlow;
};

#endif  // SOURCEMODEL__SOURCEMODELAPP_H