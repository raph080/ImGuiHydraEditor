#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <GraphEditor.h>
#include <ImGuiFileDialog.h>
#include <ImGuizmo.h>
#include <boost/predef/os.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/sphere.h>

#include <vector>

#include "editor.h"
#include "outliner.h"
#include "sessionlayer.h"
#include "view.h"
#include "viewport.h"

using namespace std;

class MainWindow {
    public:
        MainWindow(Model* model) : model(model) { ResetDefaultViews(); };

        void Update()
        {
            ImGui::DockSpaceOverViewport();

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Load ...")) {
                        ImGuiFileDialog::Instance()->OpenDialog(
                            "LoadFile", "Choose File", ".usd,.usdc,.usda",
                            ".");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Export to ...")) {
                        ImGuiFileDialog::Instance()->OpenDialog(
                            "ExportFile", "Choose File", ".usd,.usdc,.usda",
                            ".");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Objects")) {
                    if (ImGui::BeginMenu("Create")) {
                        if (ImGui::MenuItem("Camera")) CreateCamera();
                        ImGui::Separator();
                        if (ImGui::MenuItem("Capsule")) CreateCapsule();
                        if (ImGui::MenuItem("Cone")) CreateCone();
                        if (ImGui::MenuItem("Cube")) CreateCube();
                        if (ImGui::MenuItem("Cylinder")) CreateCylinder();
                        if (ImGui::MenuItem("Plane")) CreatePlane();
                        if (ImGui::MenuItem("Sphere")) CreateSphere();
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
                if (ImGui::BeginMenu("Debug")) {
                    if (ImGui::MenuItem("default file")) {
                        model->LoadUsdStage(
                            "/Users/raphaeljouretz/Downloads/"
                            "solaris_demo_files/"
                            "LargeBasketWithCarpets.usda");
                        for (auto view : views) { view->ModelChangedEvent(); }
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            vector<View*> viewsCopy(views);
            views.clear();

            for (auto view : viewsCopy) {
                view->Update();
                if (view->IsDisplayed()) views.push_back(view);
                else delete view;
            }

            if (ImGuiFileDialog::Instance()->Display("LoadFile")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    string filePath =
                        ImGuiFileDialog::Instance()->GetFilePathName();
                    model->LoadUsdStage(filePath);
                    for (auto view : views) { view->ModelChangedEvent(); }
                }
                ImGuiFileDialog::Instance()->Close();
            }

            if (ImGuiFileDialog::Instance()->Display("ExportFile")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    string filePath =
                        ImGuiFileDialog::Instance()->GetFilePathName();
                    model->GetStage()->Export(filePath, false);
                }
                ImGuiFileDialog::Instance()->Close();
            }
        }

        void ResetDefaultViews()
        {
            // delete all existing views
            for (auto view : views) { delete view; }
            views.clear();

            // // add default views
            AddView(Outliner::VIEW_TYPE);
            AddView(Editor::VIEW_TYPE);
            AddView(SessionLayer::VIEW_TYPE);
            AddView(Viewport::VIEW_TYPE);
        }

        void AddView(const string viewType)
        {
            // count the occurences of views of type 'viewType'
            int occ = 1;
            for (auto view : views) {
                if (view->GetViewType() == viewType) occ++;
            }

            // create the unique label of the new view
            string viewLabel = viewType;
            if (occ > 1) viewLabel += " " + to_string(occ);

            if (viewType == Editor::VIEW_TYPE) {
                views.push_back(new Editor(model, viewLabel));
            }
            else if (viewType == Outliner::VIEW_TYPE) {
                views.push_back(new Outliner(model, viewLabel));
            }
            else if (viewType == SessionLayer::VIEW_TYPE) {
                views.push_back(new SessionLayer(model, viewLabel));
            }
            else if (viewType == Viewport::VIEW_TYPE) {
                views.push_back(new Viewport(model, viewLabel));
            }
        }

    private:
        vector<View*> views;
        Model* model;

        string GetNextAvailableIndexedPath(string primPath)
        {
            UsdStageRefPtr stage = model->GetStage();
            UsdPrim prim;
            int i = -1;
            string newPath;
            do {
                i++;
                if (i == 0) newPath = primPath;
                else newPath = primPath + std::to_string(i);
                prim = stage->GetPrimAtPath(SdfPath(newPath));
            } while (prim.IsValid());
            return newPath;
        }

        void CreateCamera()
        {
            string primPath = GetNextAvailableIndexedPath("/camera");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomCamera::Define(stage, SdfPath(primPath));
        }

        void CreateCapsule()
        {
            string primPath = GetNextAvailableIndexedPath("/capsule");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomCapsule::Define(stage, SdfPath(primPath));
        }
        void CreateCone()
        {
            string primPath = GetNextAvailableIndexedPath("/cone");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomCone::Define(stage, SdfPath(primPath));
        }
        void CreateCube()
        {
            string primPath = GetNextAvailableIndexedPath("/cube");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomCube::Define(stage, SdfPath(primPath));
        }

        void CreateCylinder()
        {
            string primPath = GetNextAvailableIndexedPath("/cylinder");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomCylinder::Define(stage, SdfPath(primPath));
        }

        void CreatePlane()
        {
            string primPath = GetNextAvailableIndexedPath("/plane");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomPlane::Define(stage, SdfPath(primPath));
        }

        void CreateSphere()
        {
            string primPath = GetNextAvailableIndexedPath("/sphere");
            UsdStageRefPtr stage = model->GetStage();
            UsdGeomSphere::Define(stage, SdfPath(primPath));
        }
};