#include "Application.h"

#include <embeds/font_faSolid.h>
#include <embeds/font_roboto.h>
#include <embeds/font_vollkorn.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/freetype/imgui_freetype.h>

#include <iostream>

#include "math/utils.h"

Application::Application(const std::string& title, const int initialWidth,
                         const int initialHeight)
    : m_title(title),
      m_width(initialWidth),
      m_height(initialHeight),
      m_glslVersion(nullptr),
      m_window(nullptr),
      m_pendingIsDarkTheme(true),
#ifdef SOURCEMODEL_DEBUG
      m_doShowDebugMenu(true),
      m_doShowFps(true),
      m_doShowMetrics(false)
#else
      m_doShowDebugMenu(false),
      m_doShowFps(false),
      m_doShowMetrics(false)
#endif
{
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

    contentScaleChanged.connect([this](const float contentScale) {
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
    if (!fuzzyEquals(m_contentScale, contentScale)) {
        m_contentScale = contentScale;
        contentScaleChanged(contentScale);
    }
}

bool Application::isDarkTheme() const { return m_isDarkTheme; }

void Application::setTheme(const bool isDarkTheme) {
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

float Application::em() const { return 20.0f * m_contentScale; }

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

void Application::renderMenuBar() {}

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

#ifdef __EMSCRIPTEN__
#else
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
    static ImWchar rangesRoboto[] = {
        0x0020, 0x00FF,  // Basic Latin + Latin Supplement
        0x03B1, 0x03B1,  // Greek small letter alpha
        0x2080, 0x2085,  // Subscript 0 to 5
        0x2090, 0x2090,  // Subscript a
        0x2098, 0x2098,  // Subscript m
        0x25A1, 0x25A1,  // Unknown character symbol
        0,
    };
    static ImWchar rangesVollkorn[] = {
        0x2208,
        0x2208,  // Element of
        0,
    };
    static ImWchar rangesFaSolid[] = {
        0xF04B,  // Play
        0xF04C,  // Pause
        0,
    };

    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    io.Fonts->AddFontFromMemoryTTF(embedded::gFontRoboto, embedded::gFontRoboto.size(),
                                   em(), &config, &rangesRoboto[0]);

    config.MergeMode = true;
    config.GlyphOffset.y = 1;
    io.Fonts->AddFontFromMemoryTTF(embedded::gFontVollkorn,
                                   embedded::gFontVollkorn.size(), em(), &config,
                                   &rangesVollkorn[0]);

    config.MergeMode = true;
    config.GlyphMinAdvanceX = em() + 1;
    config.GlyphOffset.y = 2;
    io.Fonts->AddFontFromMemoryTTF(embedded::gFontFaSolid, embedded::gFontFaSolid.size(),
                                   em() + 1, &config, &rangesFaSolid[0]);

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
                         ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMainMenuBar()) {
            renderMenuBar();

            if (ImGui::BeginMenu("View")) {
                // Toggle for dark/light theme
                bool currentIsDarkTheme(m_isDarkTheme);
                if (ImGui::MenuItem("Dark theme", nullptr, &currentIsDarkTheme)) {
                    m_pendingIsDarkTheme = currentIsDarkTheme;
                }

#ifndef SOURCEMODEL_DEBUG
                ImGui::MenuItem("Debug menu", nullptr, &m_doShowDebugMenu);
#endif

                ImGui::EndMenu();
            }

            if (m_doShowDebugMenu) {
                ImGui::Separator();
                if (ImGui::BeginMenu("Debug")) {
                    ImGui::MenuItem("Show FPS", nullptr, &m_doShowFps);
                    ImGui::MenuItem("Show ImGui metrics", nullptr, &m_doShowMetrics);
                    ImGui::EndMenu();
                }
            }

#if defined(__EMSCRIPTEN__) && !defined(SOURCEMODEL_DEBUG)
            const char* madeByText = "Made by Clo Yun-Hee Dufour (clo-yunhee on GitHub)";
            auto        madeByTextPosX =
                ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
                ImGui::CalcTextSize(madeByText).x - ImGui::GetStyle().ItemSpacing.x;
#endif

            if (m_doShowFps) {
                // Show FPS if we're in a debug build
                std::string text =
                    "FPS: " + std::to_string((int)ImGui::GetIO().Framerate);
#if defined(__EMSCRIPTEN__) && !defined(SOURCEMODEL_DEBUG)
                auto posX = madeByTextPosX - ImGui::CalcTextSize(text.c_str()).x -
                            2 * ImGui::GetStyle().ItemSpacing.x;
#else
                auto posX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x -
                            ImGui::CalcTextSize(text.c_str()).x -
                            ImGui::GetStyle().ItemSpacing.x;
#endif
                if (posX > ImGui::GetCursorPosX()) {
                    ImGui::SetCursorPosX(posX);
                }
                ImGui::MenuItem(text.c_str(), nullptr, false, false);
            }

#if defined(__EMSCRIPTEN__) && !defined(SOURCEMODEL_DEBUG)
            if (madeByTextPosX > ImGui::GetCursorPosX())
                ImGui::SetCursorPosX(madeByTextPosX);
            ImGui::MenuItem(madeByText, nullptr, false, false);
#endif

            ImGui::EndMainMenuBar();
        }

        renderMain();
    }
    ImGui::End();
    ImGui::PopStyleVar();

    renderOther();

    if (m_doShowMetrics) {
        // Show metrics if we're in a debug build
        ImGui::ShowMetricsWindow();
    }

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

void Application::glfwErrorCallback(const int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << "\n";
}

void Application::glfwSizeCallback(GLFWwindow* window, const int width,
                                   const int height) {
    auto self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    self->m_width = width / self->m_contentScale;
    self->m_height = height / self->m_contentScale;
}

#ifndef __EMSCRIPTEN__
void Application::glfwScaleCallback(GLFWwindow* window, const float contentScale,
                                    const float) {
    auto self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    self->m_pendingContentScale = contentScale;
}
#endif

#ifdef __EMSCRIPTEN__
EM_BOOL Application::emsUiCallback(const int eventType, const EmscriptenUiEvent* uiEvent,
                                   void* userData) {
    auto app = static_cast<Application*>(userData);
    if (eventType == EMSCRIPTEN_EVENT_RESIZE) {
        app->m_width = uiEvent->windowInnerWidth;
        app->m_height = uiEvent->windowInnerHeight;
        emscripten_set_element_css_size("#canvas", app->m_width, app->m_height);
        // Force it to behave as if content scale is reset.
        app->m_contentScale = -1;
        app->m_pendingContentScale = app->queryContentScale();
        return EM_TRUE;
    }
    return EM_FALSE;
}
#endif