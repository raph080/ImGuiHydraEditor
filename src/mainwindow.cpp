#include "mainwindow.h"

#include <ImGuiFileDialog.h>
#include <imgui.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/sphere.h>

#include "views/editor.h"
#include "views/outliner.h"
#include "views/sessionlayer.h"
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
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New scene")) {
                _model->SetEmptyStage();
                for (auto view : _views) { view->ModelChangedEvent(); }
            }

            if (ImGui::MenuItem("Load ...")) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadFile", "Choose File", ".usd,.usdc,.usda", ".");
            }

            if (ImGui::MenuItem("Export to ...")) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ExportFile", "Choose File", ".usd,.usdc,.usda", ".");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Objects")) {
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::MenuItem("Camera")) _CreateCamera();
                ImGui::Separator();
                if (ImGui::MenuItem("Capsule")) _CreateCapsule();
                if (ImGui::MenuItem("Cone")) _CreateCone();
                if (ImGui::MenuItem("Cube")) _CreateCube();
                if (ImGui::MenuItem("Cylinder")) _CreateCylinder();
                if (ImGui::MenuItem("Plane")) _CreatePlane();
                if (ImGui::MenuItem("Sphere")) _CreateSphere();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::BeginMenu("Add")) {
                if (ImGui::MenuItem(Editor::VIEW_TYPE.c_str()))
                    AddView(Editor::VIEW_TYPE);
                if (ImGui::MenuItem(Outliner::VIEW_TYPE.c_str()))
                    AddView(Outliner::VIEW_TYPE);
                if (ImGui::MenuItem(SessionLayer::VIEW_TYPE.c_str()))
                    AddView(SessionLayer::VIEW_TYPE);
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

    if (ImGuiFileDialog::Instance()->Display("LoadFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _model->LoadUsdStage(filePath);
            for (auto view : _views) { view->ModelChangedEvent(); }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("ExportFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _model->GetStage()->Export(filePath, false);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void MainWindow::ResetDefaultViews()
{
    // delete all existing views
    for (auto view : _views) { delete view; }
    _views.clear();

    // // add default views
    AddView(Outliner::VIEW_TYPE);
    AddView(Editor::VIEW_TYPE);
    AddView(SessionLayer::VIEW_TYPE);
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
    else if (viewType == SessionLayer::VIEW_TYPE) {
        _views.push_back(new SessionLayer(_model, viewLabel));
    }
    else if (viewType == Viewport::VIEW_TYPE) {
        _views.push_back(new Viewport(_model, viewLabel));
    }
}

string MainWindow::_GetNextAvailableIndexedPath(string primPath)
{
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdPrim prim;
    int i = -1;
    string newPath;
    do {
        i++;
        if (i == 0) newPath = primPath;
        else newPath = primPath + to_string(i);
        prim = stage->GetPrimAtPath(pxr::SdfPath(newPath));
    } while (prim.IsValid());
    return newPath;
}

void MainWindow::_CreateCamera()
{
    string primPath = _GetNextAvailableIndexedPath("/camera");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomCamera::Define(stage, pxr::SdfPath(primPath));
}

void MainWindow::_CreateCapsule()
{
    string primPath = _GetNextAvailableIndexedPath("/capsule");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomCapsule::Define(stage, pxr::SdfPath(primPath));
}
void MainWindow::_CreateCone()
{
    string primPath = _GetNextAvailableIndexedPath("/cone");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomCone::Define(stage, pxr::SdfPath(primPath));
}
void MainWindow::_CreateCube()
{
    string primPath = _GetNextAvailableIndexedPath("/cube");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomCube::Define(stage, pxr::SdfPath(primPath));
}

void MainWindow::_CreateCylinder()
{
    string primPath = _GetNextAvailableIndexedPath("/cylinder");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomCylinder::Define(stage, pxr::SdfPath(primPath));
}

void MainWindow::_CreatePlane()
{
    string primPath = _GetNextAvailableIndexedPath("/plane");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomPlane::Define(stage, pxr::SdfPath(primPath));
}

void MainWindow::_CreateSphere()
{
    string primPath = _GetNextAvailableIndexedPath("/sphere");
    pxr::UsdStageRefPtr stage = _model->GetStage();
    pxr::UsdGeomSphere::Define(stage, pxr::SdfPath(primPath));
}
