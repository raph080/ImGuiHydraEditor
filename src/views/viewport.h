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
#include <pxr/usd/usd/prim.h>

#include "engine.h"
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
         * @brief Override of the View::_GetGizmoWindowFlags
         *
         */
        ImGuiWindowFlags _GetGizmoWindowFlags() override;

    private:
        const float _FREE_CAM_FOV = 45.f;
        const float _FREE_CAM_NEAR = 0.1f;
        const float _FREE_CAM_FAR = 10000.f;

        bool _isAmbientLightEnabled, _isDomeLightEnabled, _isGridEnabled;
        pxr::SdfPath _activeCam;

        pxr::GfVec3d _eye, _at, _up;
        pxr::GfMatrix4d _proj;

        Engine* _engine;
        ImGuiWindowFlags _gizmoWindowFlags;

        ImGuizmo::OPERATION _curOperation;
        ImGuizmo::MODE _curMode;

        /**
         * @brief Get the width of the viewport
         *
         * @return the width of the viewport
         */
        float _GetViewportWidth();

        /**
         * @brief Get the height of the viewport
         *
         * @return the height of the viewport
         */
        float _GetViewportHeight();

        /**
         * @brief Override of the View::Draw
         *
         */
        void _Draw() override;

        /**
         * @brief Draw the Menu bar of the viewport
         *
         */
        void _DrawMenuBar();

        /**
         * @brief Configure ImGuizmo
         *
         */
        void _ConfigureImGuizmo();

        /**
         * @brief Update the grid within the viewport
         *
         */
        void _UpdateGrid();

        /**
         * @brief Update the USD render
         *
         */
        void _UpdateHydraRender();

        /**
         * @brief Update the transform Guizmo (the 3 axis of a selection)
         *
         */
        void _UpdateTransformGuizmo();

        /**
         * @brief Update the gizmo cube (cube at top right)
         *
         */
        void _UpdateCubeGuizmo();

        /**
         * @brief Update the label of the current Usd Hydra plugin used by the
         * viewport (above the gizmo cube)
         *
         */
        void _UpdatePluginLabel();

        /**
         * @brief Pan the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the pan
         */
        void _PanActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Orbit the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the orbit
         */
        void _OrbitActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Zoom the active camera by the mouse position delta
         *
         * @param mouseDeltaPos the mouse position delta for the zoom
         */
        void _ZoomActiveCam(ImVec2 mouseDeltaPos);

        /**
         * @brief Zoom the active calera by the scroll whell value
         *
         * @param scrollWheel the scroll wheel value for the zoom
         */
        void _ZoomActiveCam(float scrollWheel);

        /**
         * @brief Set the free camera as the active one
         *
         */
        void _SetFreeCamAsActive();

        /**
         * @brief Set the given camera as the active one
         *
         * @param primPath the path to the camera to set as active
         */
        void _SetActiveCam(pxr::SdfPath primPath);

        /**
         * @brief Update the viewport from the active camera
         *
         */
        void _UpdateViewportFromActiveCam();

        /**
         * @brief Get the view matrix of the viewport
         *
         * @return the view matrix
         */
        pxr::GfMatrix4d _getCurViewMatrix();

        /**
         * @brief Update active camera from viewport
         *
         */
        void _UpdateActiveCamFromViewport();

        /**
         * @brief Update Viewport projection matrix from the active camera
         *
         */
        void _UpdateProjection();

        /**
         * @brief Focus the active camera and the viewport on the given prim
         *
         * @param primPath the prim to focus on
         */
        void _FocusOnPrim(pxr::SdfPath primPath);

        /**
         * @brief Override of the View::_KeyPressEvent
         *
         */
        void _KeyPressEvent(ImGuiKey key) override;

        /**
         * @brief Override of the View::_MouseMoveEvent
         *
         */
        void _MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos) override;

        /**
         * @brief Override of the View::_MouseReleaseEvent
         *
         */
        void _MouseReleaseEvent(ImGuiMouseButton_ button,
                                ImVec2 mousePos) override;

        /**
         * @brief Override of the View::_HoverInEvent
         *
         */
        void _HoverInEvent() override;

        /**
         * @brief Override of the View::_HoverOutEvent
         *
         */
        void _HoverOutEvent() override;
};