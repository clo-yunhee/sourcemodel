#ifndef SOURCEMODEL__PLATFORM_H
#define SOURCEMODEL__PLATFORM_H

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
#else
    #include <glad/glad.h>
#endif

#include <glfw/glfw3.h>
#include <imgui.h>

#include <optional>
#include <sigslot/signal.hpp>
#include <string>

class Application {
   public:
    Application(const std::string &title, int initialWidth = 1280,
                int initialHeight = 720);
    virtual ~Application();

    float                  contentScale() const;
    void                   setContentScale(float);
    sigslot::signal<float> contentScaleChanged;

    bool                  isDarkTheme() const;
    void                  setTheme(bool isDarkTheme);
    sigslot::signal<bool> themeChanged;

    void start();
    void resetDeviceObjects();

   protected:
    virtual void setupThemeColors(ImGuiStyle &style);
    virtual void setupThemeSizes(ImGuiStyle &style);

    virtual void renderMain();
    virtual void renderOther();

   private:
    bool initGLFW();
    bool loadGLFunctions();
    bool setupImGui();

    void setupFonts();
    void setupTheme();

    void mainLoopBody();

    float queryContentScale();

   private:
#ifdef __EMSCRIPTEN__
    // Emscripten-specific callbacks
    static EM_BOOL emsUiCallback(int eventType, const EmscriptenUiEvent *uiEvent,
                                 void *userData);
#endif

    // GLFW callbacks
    static void glfwErrorCallback(int error, const char *description);
    static void glfwSizeCallback(GLFWwindow *window, int width, int height);
#ifndef __EMSCRIPTEN__
    static void glfwScaleCallback(GLFWwindow *window, float xscale, float yscale);
#endif

    std::string m_title;
    float       m_width;
    float       m_height;

    const char *m_glslVersion;
    GLFWwindow *m_window;

    std::optional<float> m_pendingContentScale;
    float                m_contentScale;

    std::optional<float> m_pendingIsDarkTheme;
    bool                 m_isDarkTheme;
};

#endif  // SOURCEMODEL__PLATFORM_H