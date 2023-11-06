/**
 * @file viewport.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Viewport view acts as a viewport for the data within the current
 * Model. It allows to visualize the USD data with the current Model using an
 * instance of UsdImagingGLEngine.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
// clang-format off
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

/**
 * @brief Viewport view acts as a viewport for the data within the current
 * Model. It allows to visualize the USD data with the current Model using an
 * instance of UsdImagingGLEngine.
 *
 */
class Viewport : public View {
    public:
        inline static const string VIEW_TYPE = "Viewport";

        /**
         * @brief Construct a new Viewport object
         *
         * @param model the Model of the new Viewport view
         * @param label the ImGui label of the new Viewport view
         */
        Viewport(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Destroy the Viewport object
         *
         */
        ~Viewport();

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

        /**
         * @brief Override of the View::GetGizmoWindowFlags
         *
         */
        ImGuiWindowFlags GetGizmoWindowFlags() override;

        /**
         * @brief Override of the View::ModelChangeEvent
         *
         */
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
        pxr::TfToken _curPlugin;

        ImGuizmo::OPERATION _curOperation;
        ImGuizmo::MODE _curMode;

        /**
         * @brief Get the width of the viewport
         *
         * @return the width of the viewport
         */
        float GetViewportWidth();

        /**
         * @brief Get the height of the viewport
         *
         * @return the height of the viewport
         */
        float GetViewportHeight();

        /**
         * @brief Override of the View::Draw
         *
         */
        void Draw() override;

        /**
         * @brief Draw the Menu bar of the viewport
         *
         */
        void DrawMenuBar();

        /**
         * @brief Configure ImGuizmo
         *
         */
        void ConfigureImGuizmo();

        /**
         * @brief Update the grid within the viewport
         *
         */
        void UpdateGrid();

        /**
         * @brief Update the USD render
         *
         */
        void UpdateUsdRender();

        /**
         * @brief Update the transform Guizmo (the 3 axis of a selection)
         *
         */
        void UpdateTransformGuizmo();

        /**
         * @brief Update the gizmo cube (cube at top right)
         *
         */
        void UpdateCubeGuizmo();

        /**
         * @brief Update the label of the current Usd Hydra plugin used by the
         * viewport (above the gizmo cube)
         *
         */
        void UpdatePluginLabel();

        /**
         * @brief Pan the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the pan
         */
        void PanActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Orbit the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the orbit
         */
        void OrbitActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Zoom the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the zoom
         */
        void ZoomActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Zoom the active calera by the scroll whell value
         *
         * @param scrollWheel the scroll wheel value for the zoom
         */
        void ZoomActiveCam(float scrollWheel);

        /**
         * @brief Set the free camera as the active one
         *
         */
        void SetFreeCamAsActive();

        /**
         * @brief Set the given camera as the active one
         *
         * @param cam the camera to set as active
         */
        void SetActiveCam(pxr::UsdPrim cam);

        /**
         * @brief Update the viewport from the active camera
         *
         */
        void UpdateViewportFromActiveCam();

        /**
         * @brief Get the view matrix of the viewport
         *
         * @return the view matrix
         */
        pxr::GfMatrix4d getCurViewMatrix();

        /**
         * @brief Update active camera from viewport
         *
         */
        void UpdateActiveCamFromViewport();

        /**
         * @brief Update Viewport projection matrix from the active camera
         *
         */
        void UpdateProjection();

        /**
         * @brief Focus the active camera and the viewport on the given prim
         *
         * @param prim the prim to focus on
         */
        void FocusOnPrim(pxr::UsdPrim prim);

        /**
         * @brief Override of the View::KeyPressEvent
         *
         */
        void KeyPressEvent(ImGuiKey key) override;

        /**
         * @brief Override of the View::MouseMoveEvent
         *
         */
        void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos) override;

        /**
         * @brief Override of the View::MouseReleaseEvent
         *
         */
        void MouseReleaseEvent(ImGuiMouseButton_ button,
                               ImVec2 mousePos) override;

        /**
         * @brief Find the prim that is intersected by a ray casted at the
         * given position of the view
         *
         * @param pos the position to cast the ray from
         * @return pxr::UsdPrim
         */
        pxr::UsdPrim FindIntersection(ImVec2 pos);

        /**
         * @brief Override of the View::HoverInEvent
         *
         */
        void HoverInEvent() override;

        /**
         * @brief Override of the View::HoverOutEvent
         *
         */
        void HoverOutEvent() override;
};