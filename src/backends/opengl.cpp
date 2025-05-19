#include "backend.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "pxr/imaging/hgiGL/texture.h"
#include "pxr/imaging/hgi/texture.h"

#include <cstdint>
#include <iostream>

static GLFWwindow* window = nullptr;

/**
 * @brief Initialize Glfw
 *
 * @return a pointer to the current GL context
 */
GLFWwindow* InitGlfw(const char* title)
{
    // Initialize GLFW
    if (!glfwInit()) return NULL;

    int width = 1280;
    int height = 720;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);             // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac


    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    return window;
}

/**
 * @brief initialize Glew
 *
 * @return true if Glew was successfully initialized
 * @return false otherwise
 */
bool InitGlew()
{
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief initialize ImGui
 *
 * @param window the current GL context
 * @return true if ImGui was successfully initialized
 * @return false otherwise
 */
bool InitImGui(GLFWwindow* window)
{
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking

    const char* glsl_version = "#version 150";

    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    return true;
}

/**
 * @brief Terminate Glfw
 *
 * @param window the current GL context
 */
void TerminateGlfw(GLFWwindow* window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

/**
 * @brief Terminate ImGui
 *
 */
void TerminateImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(NULL);
}

void InitBackend()
{
    const char* TITLE = "ImGui Hydra Editor";

    window = InitGlfw(TITLE);
    if (!window || !InitGlew() || !InitImGui(window)){
        std::cout << "Failed to initialize the window" << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
}

void ShutdownBackend()
{
    TerminateImGui();
    TerminateGlfw(window);
}

bool ShouldCloseApp()
{
    return glfwWindowShouldClose(window);
}

void PollEvents()
{
    glfwPollEvents();
}

bool BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    return true;
}

void EndFrame()
{
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

void *GetPointerToHgiTextureBackend(pxr::HgiTextureHandle texHandle, float width, float height)
{
    pxr::HgiGLTexture* srcTexture = static_cast<pxr::HgiGLTexture*>(texHandle.Get());

    return (void*) srcTexture->GetTextureId();
}
