// main.cpp

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "layouts/layout.h"
#include "mainwindow.h"
#include "models/model.h"
#include "style/imgui_spectrum.h"


#include <iostream>
#include "backends/backend.h"

int main(int argc, const char** argv)
{
    InitBackend();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::Spectrum::StyleColorsSpectrum();

    LoadDefaultOrCustomLayout();

    pxr::Model model;
    pxr::MainWindow mainWindow(&model);

    while (!ShouldCloseApp()) {
        PollEvents();
        if (!BeginFrame()) continue;

        ImGui::NewFrame();

        mainWindow.Update();

        ImGui::Render();

        EndFrame();
    }

    ShutdownBackend();
    return 0;
}