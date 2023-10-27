/**
 * @file main.cpp
 * @author Raphael Jouretz (rjouretz.com)
 * @brief main file containing the main function.
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <boost/predef/os.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <iostream>
#include <ostream>

#include "layouts/layout.h"
#include "mainwindow.h"
#include "models/model.h"
#include "style/imgui_spectrum.h"

// what is currently not working yet
// using gizmo cube when z axis is up (y axis hardcoded in imguizmo)
// ImGui::InputTextMultiline does not yet allow horizontal scroll
// does not support instances editing
// imguizmo bug: guizmo not working if multiple viewport and selection empty

// switch to HdEngine or HdxTaskController
//    allow multi scene delegate
//    add the grid 'dynamically' to the renderindex
//    abstract model interface to hydra prim (nor usd stage or prim)

// selection is single
// no animation supported (time=default)
// fake events
// instanceable not fully handled (not visible in outliner and selection in
// viewport is switched to instanceable parent if instance is selected )
// no undo
// grid is bottom layer (not hydra geom)
// program does not keep multiple instances of same view when relaunched

// print current version of submodules in doc

// explain how to navigate in viewport

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

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);             // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

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

    ImGui::Spectrum::StyleColorsSpectrum();

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif

    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    LoadDefaultOrCustomLayout();

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

/**
 * @brief Main function
 *
 */
int main(int argc, char** argv)
{
    const char* TITLE = "ImGui Hydra Editor";

    GLFWwindow* window = InitGlfw(TITLE);
    if (!window || !InitGlew() || !InitImGui(window)) return 1;

    glEnable(GL_DEPTH_TEST);

    Model model;
    if (argc > 1) model.LoadUsdStage(argv[1]);

    MainWindow mainWindow(&model);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        mainWindow.Update();

        ImGui::Render();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    TerminateImGui();
    TerminateGlfw(window);

    return 0;
}