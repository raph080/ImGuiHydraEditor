#include "mainwindow.h"

#include <imgui.h>

#include "views/editor.h"
#include "views/outliner.h"
#include "views/usdsessionlayer.h"
#include "views/view.h"
#include "views/viewport.h"

MainWindow::MainWindow(Model* model) : _model(model)
{
    ResetDefaultViews();
};

void MainWindow::Update()
{
    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::BeginMenu("Add")) {
                if (ImGui::MenuItem(Editor::VIEW_TYPE.c_str()))
                    AddView(Editor::VIEW_TYPE);
                if (ImGui::MenuItem(Outliner::VIEW_TYPE.c_str()))
                    AddView(Outliner::VIEW_TYPE);
                if (ImGui::MenuItem(UsdSessionLayer::VIEW_TYPE.c_str()))
                    AddView(UsdSessionLayer::VIEW_TYPE);
                if (ImGui::MenuItem(Viewport::VIEW_TYPE.c_str()))
                    AddView(Viewport::VIEW_TYPE);

                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Default views")) ResetDefaultViews();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    vector<View*> viewsCopy(_views);
    _views.clear();

    for (auto view : viewsCopy) {
        view->Update();
        if (view->IsDisplayed()) _views.push_back(view);
        else delete view;
    }
}

void MainWindow::ResetDefaultViews()
{
    // delete all existing views
    for (auto view : _views) { delete view; }
    _views.clear();

    // // add default views
    AddView(UsdSessionLayer::VIEW_TYPE);
    AddView(Outliner::VIEW_TYPE);
    AddView(Editor::VIEW_TYPE);
    AddView(Viewport::VIEW_TYPE);
}

void MainWindow::AddView(const string viewType)
{
    // count the occurences of views of type 'viewType'
    int occ = 1;
    for (auto view : _views) {
        if (view->GetViewType() == viewType) occ++;
    }

    // create the unique label of the new view
    string viewLabel = viewType;
    if (occ > 1) viewLabel += " " + to_string(occ);

    if (viewType == Editor::VIEW_TYPE) {
        _views.push_back(new Editor(_model, viewLabel));
    }
    else if (viewType == Outliner::VIEW_TYPE) {
        _views.push_back(new Outliner(_model, viewLabel));
    }
    else if (viewType == UsdSessionLayer::VIEW_TYPE) {
        _views.push_back(new UsdSessionLayer(_model, viewLabel));
    }
    else if (viewType == Viewport::VIEW_TYPE) {
        _views.push_back(new Viewport(_model, viewLabel));
    }
}
