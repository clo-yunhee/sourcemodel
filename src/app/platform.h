#ifndef SOURCEMODEL_PLATFORM_H
#define SOURCEMODEL_PLATFORM_H

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
#else
    #include <glad/glad.h>
#endif

#include <glfw/glfw3.h>
#include <imgui.h>

struct PlatformState {
    GLFWwindow *window;
    int         width;
    int         height;
    float       contentScale;

    bool dirtyStyle;
    bool styleDark;
};

bool InitPlatform(PlatformState *state, const char *title, int initialWidth = 800,
                  int initialHeight = 600);

void PlatformMainLoop(PlatformState *state);

void RunPlatformMainLoop(PlatformState *state);

void CleanupPlatform(PlatformState *state);

bool LoadPlatformGL();

/* Returns true if content scale has changed. */
bool GetPlatformContentScale(PlatformState *state);

/* Called when platform content scale has changed. */
void PlatformUpdateFonts(PlatformState *state);

#endif  // SOURCEMODEL_PLATFORM_H