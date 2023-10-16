#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
// clang-format off
#include <boost/predef/os.h>
#include <imgui.h>
#include <imgui_internal.h>
// clang-format on
#include <ImGuizmo.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "models/model.h"
#include "utils/usd.h"
#include "view.h"

using namespace std;

class Viewport : public View {
    public:
        inline static const string VIEW_TYPE = "Viewport";

        Viewport(Model* model, const string label = VIEW_TYPE);
        ~Viewport();
        const string GetViewType() override;
        ImGuiWindowFlags GetGizmoWindowFlags() override;
        void ModelChangedEvent() override;

    private:
        const float _FREE_CAM_FOV = 45.f;
        const float _FREE_CAM_NEAR = 0.1f;
        const float _FREE_CAM_FAR = 10000.f;

        bool _isAmbientLightEnabled, _isDomeLightEnabled, _isGridEnabled;
        pxr::UsdPrim _activeCam;

        pxr::GfVec3d _eye, _at, _up;
        pxr::GfMatrix4d _proj;

        pxr::GlfDrawTargetRefPtr _drawTarget;
        ImGuiWindowFlags _gizmoWindowFlags;
        pxr::UsdImagingGLEngine* _renderer;
        pxr::UsdImagingGLRenderParams _renderParams;

        ImGuizmo::OPERATION _curOperation;
        ImGuizmo::MODE _curMode;

        float GetViewportWidth();
        float GetViewportHeight();

        void Draw() override;

        void DrawMenuBar();

        void ConfigureImGuizmo();

        void UpdateGrid();

        void UpdateRender();

        void UpdateTransformGuizmo();

        void UpdateCubeGuizmo();

        void PanActiveCam(ImVec2 mouseDeltaPos);

        void OrbitActiveCam(ImVec2 mouseDeltaPos);

        void ZoomActiveCam(ImVec2 mouseDeltaPos);

        void ZoomActiveCam(float scrollWheel);

        void SetFreeCam();

        void SetActiveCam(pxr::UsdPrim cam);

        void ReadFromActiveCam();

        pxr::GfMatrix4d getCurViewMatrix();

        void UpdateActiveCam();

        void UpdateCamProjection();

        void FocusActiveCamOnPrim(pxr::UsdPrim prim);

        void KeyPressEvent(ImGuiKey key) override;

        void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos) override;

        void MouseReleaseEvent(ImGuiMouseButton_ button,
                               ImVec2 mousePos) override;

        pxr::UsdPrim FindIntersection(ImVec2 screenPos);

        void HoverInEvent() override;
        void HoverOutEvent() override;
};