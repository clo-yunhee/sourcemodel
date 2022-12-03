#include "platform.h"

#include <embeds/font_whitney.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/freetype/imgui_freetype.h>

#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
static EM_BOOL em_ui_callback(int eventType, const EmscriptenUiEvent* uiEvent,
                              void* userData);
#endif

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << "\n";
}

#ifndef __EMSCRIPTEN__
static void glfw_scale_callback(GLFWwindow* window, float xscale, float yscale);
#endif

bool InitPlatform(PlatformState* state, const char* title, int initialWidth,
                  int initialHeight) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

#ifdef __EMSCRIPTEN__
    // Ignore initialWidth and initialHeight on Emscripten, fit the page.
    initialWidth = EM_ASM_INT(return window.innerWidth * window.devicePixelRatio);
    initialHeight = EM_ASM_INT(return window.innerHeight * window.devicePixelRatio);
#endif

    // Create window with graphics context
    GLFWwindow* window =
        glfwCreateWindow(initialWidth, initialHeight, title, nullptr, nullptr);
    if (window == nullptr) {
        return false;
    }
    state->window = window;
    glfwSetWindowUserPointer(window, state);
    glfwMakeContextCurrent(window);
#ifndef __EMSCRIPTEN__
    // Enable vsync. This call is replaced on Emscripten by using a RAF main loop.
    glfwSwapInterval(1);
#endif

    LoadPlatformGL();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
#ifdef IMGUI_ENABLE_FREETYPE
    io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
#endif

#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt
    // to do a fopen() of the imgui.ini file. You may manually call
    // LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;
#endif

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Force get content scale one time.
    state->contentScale = -1.0f;
    GetPlatformContentScale(state);

#ifndef __EMSCRIPTEN__
    // Register content scale events.
    glfwSetWindowContentScaleCallback(window, glfw_scale_callback);
#endif

#ifdef __EMSCRIPTEN__
    // Register HTML5 events on Emscripten.
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, state, true,
                                   em_ui_callback);
#endif

    return true;
}

void PlatformMainLoop(PlatformState* state) {
    glfwPollEvents();

    if (state->dirtyStyle) {
        ImGuiStyle& style = ImGui::GetStyle();
        style = ImGuiStyle();
        if (state->styleDark) {
            ImGui::StyleColorsDark();
            style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.0f);
        } else {
            ImGui::StyleColorsLight();
            style.Colors[ImGuiCol_Text] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
        }
        style.WindowRounding = 3.0f;
        style.ScaleAllSizes(state->contentScale);
        PlatformUpdateFonts(state);
        state->dirtyStyle = false;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    if (ImGui::Begin("Main", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        /// TODO: call subroutine for rendering
        ImGui::TextUnformatted("Hello, world!");
        ImGui::TextUnformatted("Hello, world!");
        ImGui::TextUnformatted("Hello, world!");

        // Toggle for dark/light theme
        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::GetFrameHeight() -
                             ImGui::GetStyle().ItemSpacing.y);
        if (ImGui::Checkbox("Dark theme", &state->styleDark)) {
            state->dirtyStyle = true;
        }

        // Show FPS if we're in a debug build
#ifndef NDEBUG
        std::string text = "FPS: " + std::to_string((int)ImGui::GetIO().Framerate);
        auto        posX = (ImGui::GetCursorPosX() + ImGui::GetContentRegionMax().x -
                     ImGui::CalcTextSize(text.c_str()).x - ImGui::GetScrollX() -
                     2 * ImGui::GetStyle().ItemSpacing.x);
        if (posX > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(posX);
        ImGui::SetCursorPosY(2 * ImGui::GetStyle().ItemSpacing.y);
        ImGui::TextUnformatted(text.c_str());
#endif
    }
    ImGui::End();
    ImGui::PopStyleVar();

    /// TODO: call subroutine for rendering
    if (ImGui::Begin("Child window test")) {
        ImGui::TextUnformatted("Child window.");
    }
    ImGui::End();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, state->width * state->contentScale,
               state->height * state->contentScale);
    ImVec4 clearColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
                 clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(state->window);
}

void RunPlatformMainLoop(PlatformState* state) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
        [](void* arg) -> void { PlatformMainLoop((PlatformState*)arg); }, state, 0, true);
#else
    while (!glfwWindowShouldClose(state->window)) PlatformMainLoop(state);
#endif
}

void CleanupPlatform(PlatformState* state) {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(state->window);
    glfwTerminate();
}

bool LoadPlatformGL() {
#ifdef __EMSCRIPTEN__
#else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return false;
    }
#endif
    return true;
}

bool GetPlatformContentScale(PlatformState* state) {
    float contentScale;
#ifdef __EMSCRIPTEN__
    // On Emscripten, we get the content scale from the Emscripten API.
    contentScale = emscripten_get_device_pixel_ratio();
#else
    glfwGetWindowContentScale(state->window, &contentScale, nullptr);
#endif

    if (fabs(state->contentScale - contentScale) > 1e-6) {
        state->contentScale = contentScale;
        state->dirtyStyle = true;
        return true;
    }
    return false;
}

void PlatformUpdateFonts(PlatformState* state) {
    // Setup font
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.FontBuilderFlags =
        ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting;
    // config.RasterizerMultiply = (state->styleDark ? 0.92f : 0.87f);
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryTTF(embedded::gFontWhitney, embedded::gFontWhitney.size(),
                                   17.0f * state->contentScale, &config,
                                   io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
}

#ifndef __EMSCRIPTEN__
static void glfw_scale_callback(GLFWwindow* window, float contentScale, float) {
    auto state = static_cast<PlatformState*>(glfwGetWindowUserPointer(window));
    GetPlatformContentScale(state);
}
#endif

#ifdef __EMSCRIPTEN__
static EM_BOOL em_ui_callback(int eventType, const EmscriptenUiEvent* uiEvent,
                              void* userData) {
    auto state = static_cast<PlatformState*>(userData);

    if (eventType == EMSCRIPTEN_EVENT_RESIZE) {
        GetPlatformContentScale(state);
        state->width = uiEvent->windowInnerWidth;
        state->height = uiEvent->windowInnerHeight;
        glfwSetWindowSize(state->window, state->width * state->contentScale,
                          state->height * state->contentScale);
        emscripten_set_element_css_size("#canvas", state->width, state->height);
        return EM_TRUE;
    }

    return EM_FALSE;
}
#endif