#include <embeds/font_whitney.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/freetype/imgui_freetype.h>

#include <iostream>
#include <string>

#include "platform.h"

Application::Application(const std::string& title, int initialWidth, int initialHeight)
    : m_title(title),
      m_width(initialWidth),
      m_height(initialHeight),
      m_glslVersion(nullptr),
      m_window(nullptr),
      m_isDarkTheme(true) {
#ifdef __EMSCRIPTEN__
    // Ignore initialWidth and initialHeight on Emscripten, fit the page.
    m_width = EM_ASM_INT(return window.innerWidth * window.devicePixelRatio);
    m_height = EM_ASM_INT(return window.innerHeight * window.devicePixelRatio);
#endif

    if (!initGLFW()) {
        throw std::runtime_error("GLFW initialization failed");
    }

    if (!loadGLFunctions()) {
        throw std::runtime_error("OpenGL function loading failed");
    }

    if (!setupImGui()) {
        throw std::runtime_error("ImGui setup failed");
    }

    contentScaleChanged.connect([this](float contentScale) {
        glfwSetWindowSize(m_window, m_width * contentScale, m_height * contentScale);
    });

    contentScaleChanged.connect([this](float) {
        setupTheme();
        setupFonts();
    });

    themeChanged.connect([this](bool) { setupTheme(); });

    // Run fetchContentScale one time before everything.
    setContentScale(queryContentScale());
}

Application::~Application() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

float Application::contentScale() const { return m_contentScale; }

void Application::setContentScale(const float contentScale) {
    if (fabs(m_contentScale - contentScale) > 1e-6) {
        m_contentScale = contentScale;
        contentScaleChanged(contentScale);
    }
}

bool Application::isDarkTheme() const { return m_isDarkTheme; }

void Application::setTheme(bool isDarkTheme) {
    if (m_isDarkTheme != isDarkTheme) {
        m_isDarkTheme = isDarkTheme;
        themeChanged(isDarkTheme);
    }
}

void Application::start() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
        [](void* arg) -> void { ((Application*)arg)->mainLoopBody(); }, this, 0, true);
#else
    while (!glfwWindowShouldClose(m_window)) mainLoopBody();
#endif
}

void Application::resetDeviceObjects() {
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    if (!ImGui_ImplOpenGL3_CreateDeviceObjects()) {
        throw std::runtime_error("Device objects reset failed");
    }
}

void Application::setupThemeColors(ImGuiStyle& style) {
    if (m_isDarkTheme) {
        ImGui::StyleColorsDark(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.0f);
    } else {
        ImGui::StyleColorsLight(&style);
        style.Colors[ImGuiCol_Text] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    }
}

void Application::setupThemeSizes(ImGuiStyle& style) { style.WindowRounding = 3.0f; }

void Application::renderMain() {
    ImGui::TextUnformatted("Hello, world!");
    ImGui::TextUnformatted("Placeholder main window text");
}

void Application::renderOther() {
    if (ImGui::Begin("Child window")) {
        ImGui::TextUnformatted("Placeholder child window text");
    }
    ImGui::End();
}

bool Application::initGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        return false;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    m_glslVersion = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    m_glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    m_glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);

#ifndef __EMSCRIPTEN__
    // Register content scale events.
    glfwSetWindowContentScaleCallback(m_window, glfwScaleCallback);

    // Enable vsync. This call is replaced on Emscripten by using a RAF main loop.
    glfwSwapInterval(1);
#endif

    glfwSetWindowSizeCallback(m_window, glfwSizeCallback);

    return true;
}

bool Application::loadGLFunctions() {
#ifdef __EMSCRIPTEN__
#else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }
#endif
    return true;
}

bool Application::setupImGui() {
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
    if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) return false;
    if (!ImGui_ImplOpenGL3_Init(m_glslVersion)) return false;

#ifdef __EMSCRIPTEN__
    // Register HTML5 events on Emscripten.
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true,
                                   emsUiCallback);
#endif

    return true;
}

void Application::setupFonts() {
    // Setup font
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.FontBuilderFlags =
        ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting;
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryTTF(embedded::gFontWhitney, embedded::gFontWhitney.size(),
                                   17.0f * m_contentScale, &config,
                                   io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();
    resetDeviceObjects();
}

void Application::setupTheme() {
    ImGuiStyle style;
    setupThemeColors(style);
    setupThemeSizes(style);
    style.ScaleAllSizes(m_contentScale);
    ImGui::GetStyle() = style;
}

void Application::mainLoopBody() {
    if (m_pendingIsDarkTheme) {
        setTheme(m_pendingIsDarkTheme.value());
        m_pendingIsDarkTheme.reset();
    }

    if (m_pendingContentScale) {
        setContentScale(m_pendingContentScale.value());
        m_pendingContentScale.reset();
    }

    glfwPollEvents();

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
        renderMain();

        // Toggle for dark/light theme
        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::GetFrameHeight() -
                             ImGui::GetStyle().ItemSpacing.y);
        bool currentIsDarkTheme(m_isDarkTheme);
        if (ImGui::Checkbox("Dark theme", &currentIsDarkTheme)) {
            m_pendingIsDarkTheme = currentIsDarkTheme;
        }

#ifndef NDEBUG
        // Show FPS if we're in a debug build
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

    renderOther();

#ifndef NDEBUG
    // Show metrics if we're in a debug build
    ImGui::ShowMetricsWindow();
#endif

    // Rendering
    ImGui::Render();
    glViewport(0, 0, m_width * m_contentScale, m_height * m_contentScale);
    ImVec4 clearColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
                 clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

float Application::queryContentScale() {
#ifdef __EMSCRIPTEN__
    // On Emscripten, we get the content scale from the Emscripten API.
    return emscripten_get_device_pixel_ratio();
#else
    float contentScale;
    glfwGetWindowContentScale(m_window, &contentScale, nullptr);
    return contentScale;
#endif
}

void Application::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << "\n";
}

void Application::glfwSizeCallback(GLFWwindow* window, int width, int height) {
    auto self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    self->m_width = width / self->m_contentScale;
    self->m_height = height / self->m_contentScale;
}

#ifndef __EMSCRIPTEN__
void Application::glfwScaleCallback(GLFWwindow* window, float contentScale, float) {
    auto self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    self->m_pendingContentScale = contentScale;
}
#endif

#ifdef __EMSCRIPTEN__
EM_BOOL Application::emsUiCallback(int eventType, const EmscriptenUiEvent* uiEvent,
                                   void* userData) {
    auto app = static_cast<Application*>(userData);
    if (eventType == EMSCRIPTEN_EVENT_RESIZE) {
        app->m_width = uiEvent->windowInnerWidth;
        app->m_height = uiEvent->windowInnerHeight;
        emscripten_set_element_css_size("#canvas", app->m_width, app->m_height);
        app->m_pendingContentScale = app->queryContentScale();
        return EM_TRUE;
    }
    return EM_FALSE;
}
#endif