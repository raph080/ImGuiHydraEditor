// main.cpp

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "layouts/layout.h"
#include "mainwindow.h"
#include "models/model.h"
#include "style/imgui_spectrum.h"
#include "backends/backend.h"

#include <iostream>

static pxr::Model model;
static pxr::MainWindow* mainWindow;

/**
 * @brief The function called every frame by the backend.
 */
void run()
{
    ImGui::NewFrame();
    mainWindow->Update();
    ImGui::Render();
}

int main(int argc, const char** argv)
{
    const char* TITLE = "ImGui Hydra Editor";
    int WIDTH = 1280;
    int HEIGHT = 720;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (InitBackend(TITLE, WIDTH, HEIGHT) != 0)
    {
        std::cerr << "Error while initializing backend; exiting." << std::endl;
        return -1;
    }

    ImGui::Spectrum::StyleColorsSpectrum();
    LoadDefaultOrCustomLayout();

    mainWindow = new pxr::MainWindow(&model); 

    RunBackend(run);

    ShutdownBackend();

    return 0;
}