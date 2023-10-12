#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
// clang-format off
#include <boost/predef/os.h>
#include <imgui.h>
#include <imgui_internal.h>
// clang-format on
#include <ImGuizmo.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "models/model.h"
#include "utils/usd.h"
#include "view.h"

using namespace std;

PXR_NAMESPACE_OPEN_SCOPE

class Viewport : public View {
    public:
        inline static const string VIEW_TYPE = "Viewport";

        Viewport(Model* model, const string label = VIEW_TYPE)
            : View(model, label)
        {
            _drawTarget = GlfDrawTarget::New(GfVec2i(100));
            _drawTarget->Bind();
            _drawTarget->AddAttachment(HdAovTokens->color, GL_RGBA, GL_FLOAT,
                                       GL_RGBA);
            _drawTarget->AddAttachment(HdAovTokens->depth, GL_DEPTH_COMPONENT,
                                       GL_FLOAT, GL_DEPTH_COMPONENT);
            _drawTarget->Unbind();

            _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;
            _isAmbientLightEnabled = true;
            _isDomeLightEnabled = false;
            _isGridEnabled = true;

            _curOperation = ImGuizmo::TRANSLATE;
            _curMode = ImGuizmo::LOCAL;

            _eye = GfVec3d(5, 5, 5);
            _at = GfVec3d(0, 0, 0);
            _up = GetModel()->GetUpAxis();

            UpdateActiveCam();

            _renderer = new UsdImagingGLEngine();
        };
        ~Viewport()
        {
            _drawTarget = GlfDrawTargetRefPtr();
            delete _renderer;
        }

        const string GetViewType() override { return VIEW_TYPE; };

        ImGuiWindowFlags GetGizmoWindowFlags() override
        {
            return _gizmoWindowFlags;
        };

        void ModelChangedEvent() override
        {
            delete _renderer;
            _renderer = new UsdImagingGLEngine();
            _up = GetModel()->GetUpAxis();
        };

    private:
        const float FREE_CAM_FOV = 45.f;
        const float FREE_CAM_NEAR = 0.1f;
        const float FREE_CAM_FAR = 10000.f;

        bool _isAmbientLightEnabled, _isDomeLightEnabled, _isGridEnabled;
        UsdPrim _activeCam;

        GfVec3d _eye, _at, _up;
        GfMatrix4d _proj;

        GlfDrawTargetRefPtr _drawTarget;
        ImGuiWindowFlags _gizmoWindowFlags;
        UsdImagingGLEngine* _renderer;
        UsdImagingGLRenderParams _renderParams;

        ImGuizmo::OPERATION _curOperation;
        ImGuizmo::MODE _curMode;

        float GetViewportWidth() { return GetInnerRect().GetWidth(); }
        float GetViewportHeight() { return GetInnerRect().GetHeight(); }

        void Draw() override
        {
            DrawMenuBar();

            if (GetViewportWidth() <= 0 || GetViewportHeight() <= 0) return;

            ImGui::BeginChild("GameRender");

            ConfigureImGuizmo();

            // read from active cam in case it is modify by another view
            if (!ImGui::IsWindowFocused()) ReadFromActiveCam();

            UpdateCamProjection();
            UpdateGrid();
            UpdateRender();
            UpdateTransformGuizmo();
            UpdateCubeGuizmo();

            ImGui::EndChild();
        };

        void DrawMenuBar()
        {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("transform")) {
                    if (ImGui::MenuItem("local translate")) {
                        _curOperation = ImGuizmo::TRANSLATE;
                        _curMode = ImGuizmo::LOCAL;
                    }
                    if (ImGui::MenuItem("local rotation")) {
                        _curOperation = ImGuizmo::ROTATE;
                        _curMode = ImGuizmo::LOCAL;
                    }
                    if (ImGui::MenuItem("local scale")) {
                        _curOperation = ImGuizmo::SCALE;
                        _curMode = ImGuizmo::LOCAL;
                    }
                    if (ImGui::MenuItem("global translate")) {
                        _curOperation = ImGuizmo::TRANSLATE;
                        _curMode = ImGuizmo::WORLD;
                    }
                    if (ImGui::MenuItem("global rotation")) {
                        _curOperation = ImGuizmo::ROTATE;
                        _curMode = ImGuizmo::WORLD;
                    }
                    if (ImGui::MenuItem("global scale")) {
                        _curOperation = ImGuizmo::SCALE;
                        _curMode = ImGuizmo::WORLD;
                    }

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("renderer")) {
                    // get all possible renderer plugins
                    TfTokenVector plugins = _renderer->GetRendererPlugins();
                    // get current renderer plugin
                    TfToken curPlugin = _renderer->GetCurrentRendererId();
                    for (auto plugin : plugins) {
                        bool enabled = (plugin == curPlugin);
                        if (ImGui::MenuItem(plugin.GetText(), NULL, enabled)) {
                            _renderer->SetRendererPlugin(plugin);
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("cameras")) {
                    bool enabled = (!_activeCam.IsValid());
                    if (ImGui::MenuItem("free camera", NULL, &enabled)) {
                        SetFreeCam();
                    }
                    for (UsdPrim cam : GetModel()->GetCameras()) {
                        bool enabled = (cam == _activeCam);
                        if (ImGui::MenuItem(cam.GetName().GetText(), NULL,
                                            enabled)) {
                            SetActiveCam(cam);
                        }
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("lights")) {
                    ImGui::MenuItem("ambient light", NULL,
                                    &_isAmbientLightEnabled);
                    ImGui::MenuItem("dome light", NULL, &_isDomeLightEnabled);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("show")) {
                    ImGui::MenuItem("grid", NULL, &_isGridEnabled);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
        }

        void ConfigureImGuizmo()
        {
            ImGuizmo::BeginFrame();

            // convert last label char to ID
            string label = GetViewLabel();
            ImGuizmo::SetID(int(label[label.size() - 1]));

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(GetInnerRect().Min.x, GetInnerRect().Min.y,
                              GetViewportWidth(), GetViewportHeight());
        }

        void UpdateGrid()
        {
            if (!_isGridEnabled) return;

            GfMatrix4f viewF(getCurViewMatrix());
            GfMatrix4f projF(_proj);
            GfMatrix4f identity(1);

            ImGuizmo::DrawGrid(viewF.data(), projF.data(), identity.data(),
                               10);
        }

        void UpdateRender()
        {
            UsdStageRefPtr stage = GetModel()->GetStage();
            GfMatrix4d view = getCurViewMatrix();
            float width = GetViewportWidth();
            float height = GetViewportHeight();

            _renderer->SetRendererAov(HdAovTokens->color);
            _renderer->SetRenderBufferSize(GfVec2i(width, height));
            _renderer->SetCameraState(view, _proj);
            _renderer->SetOverrideWindowPolicy(
                {false,
                 CameraUtilConformWindowPolicy::CameraUtilMatchVertically});

            // set selection
            SdfPathVector paths;
            for (auto&& prim : GetModel()->GetSelection())
                paths.push_back(prim.GetPrimPath());

            _renderer->SetSelected(paths);

            // set the framing
            auto displayWindow =
                GfRange2f(GfVec2f(0.0, 0.0), GfVec2f(width, height));
            // -1 to prevent error "dataWindow is larger than render buffer"
            auto dataWindow =
                GfRect2i(GfVec2i(0, 0), GfVec2i(width - 1, height - 1));
            _renderer->SetFraming(
                CameraUtilFraming(displayWindow, dataWindow));

            // set lighting state
            auto sceneAmbient = GfVec4f(0.01, 0.01, 0.01, 1.0);
            auto material = GlfSimpleMaterial();
            auto lights = GlfSimpleLightVector();

            if (_isAmbientLightEnabled) {
                GlfSimpleLight l;
                l.SetAmbient(GfVec4f(0, 0, 0, 0));
                l.SetPosition(GfVec4f(_eye[0], _eye[1], _eye[2], 1));
                lights.push_back(l);
            }
            if (_isDomeLightEnabled) {
                GlfSimpleLight l;
                l.SetIsDomeLight(true);
                if (GetModel()->GetUpAxis() == GfVec3d::ZAxis()) {
                    GfMatrix4d rotMat = GfMatrix4d().SetRotate(
                        GfRotation(GfVec3d::XAxis(), 90));
                    l.SetTransform(rotMat);
                }
                lights.push_back(l);
            }

            material.SetAmbient(GfVec4f(0.2, 0.2, 0.2, 1));
            material.SetSpecular(GfVec4f(0.1, 0.1, 0.1, 1));
            material.SetShininess(32.0);

            _renderer->SetLightingState(lights, material, sceneAmbient);

            // set render params
            _renderParams.frame = UsdTimeCode::Default();
            _renderParams.complexity = 1.0;
            _renderParams.drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
            _renderParams.showGuides = false;
            _renderParams.showProxy = true;
            _renderParams.showRender = false;
            _renderParams.forceRefresh = true;
            _renderParams.cullStyle =
                UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
            _renderParams.gammaCorrectColors = false;
            _renderParams.enableIdRender = false;
            _renderParams.enableSampleAlphaToCoverage = true;
            _renderParams.highlight = true;
            _renderParams.enableSceneMaterials = true;
            _renderParams.enableSceneLights = true;
            _renderParams.clearColor = GfVec4f(0.7, 0.7, 0.7, 0);

            // do the actual render in the draw target
            _drawTarget->Bind();
            _drawTarget->SetSize(GfVec2i(width, height));

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // do the render
            _renderer->Render(stage->GetPseudoRoot(), _renderParams);

            _drawTarget->Unbind();

            // create an imgui image with the drawtarget color data
            GLuint id = _drawTarget->GetAttachment(HdAovTokens->color)
                            ->GetGlTextureName();
            ImGui::Image((ImTextureID)id, ImVec2(width, height), ImVec2(0, 1),
                         ImVec2(1, 0));
        }

        void UpdateTransformGuizmo()
        {
            vector<UsdPrim> prims = GetModel()->GetSelection();
            if (prims.size() == 0 || !prims[0].IsValid()) return;

            UsdGeomGprim geom(prims[0]);

            GfMatrix4d transform = GetTransform(geom);
            GfMatrix4f transformF(transform);

            GfMatrix4d view = getCurViewMatrix();

            GfMatrix4f viewF(view);
            GfMatrix4f projF(_proj);

            ImGuizmo::Manipulate(viewF.data(), projF.data(), _curOperation,
                                 _curMode, transformF.data());

            if (transformF != GfMatrix4f(transform))
                SetTransform(geom, GfMatrix4d(transformF));
        }

        void UpdateCubeGuizmo()
        {
            ImRect innerRect = GetInnerRect();

            GfMatrix4d view = getCurViewMatrix();
            GfMatrix4f viewF(view);

            ImGuizmo::ViewManipulate(
                viewF.data(), 8.f,
                ImVec2(innerRect.Max.x - 128, innerRect.Min.y),
                ImVec2(128, 128), IM_COL32_BLACK_TRANS);

            if (viewF != GfMatrix4f(view)) {
                view = GfMatrix4d(viewF);
                GfFrustum frustum;
                frustum.SetPositionAndRotationFromMatrix(view.GetInverse());
                _eye = frustum.GetPosition();
                _at = frustum.ComputeLookAtPoint();

                UpdateActiveCam();
            }
        }

        void PanActiveCam(ImVec2 mouseDeltaPos)
        {
            GfVec3d camFront = _at - _eye;
            GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
            GfVec3d camUp = GfCross(camRight, camFront).GetNormalized();

            GfVec3d delta = camRight * -mouseDeltaPos.x / 100.f +
                            camUp * mouseDeltaPos.y / 100.f;

            _eye += delta;
            _at += delta;

            UpdateActiveCam();
        }

        void OrbitActiveCam(ImVec2 mouseDeltaPos)
        {
            GfRotation rot(_up, mouseDeltaPos.x / 2);
            GfMatrix4d rotMatrix = GfMatrix4d(1).SetRotate(rot);
            GfVec3d e = _eye - _at;
            GfVec4d vec4 = rotMatrix * GfVec4d(e[0], e[1], e[2], 1.f);
            _eye = _at + GfVec3d(vec4[0], vec4[1], vec4[2]);

            GfVec3d camFront = _at - _eye;
            GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
            rot = GfRotation(camRight, mouseDeltaPos.y / 2);
            rotMatrix = GfMatrix4d(1).SetRotate(rot);
            e = _eye - _at;
            vec4 = rotMatrix * GfVec4d(e[0], e[1], e[2], 1.f);
            _eye = _at + GfVec3d(vec4[0], vec4[1], vec4[2]);

            UpdateActiveCam();
        }

        void ZoomActiveCam(ImVec2 mouseDeltaPos)
        {
            GfVec3d camFront = (_at - _eye).GetNormalized();
            _eye += camFront * mouseDeltaPos.x / 100.f;

            UpdateActiveCam();
        }

        void ZoomActiveCam(float scrollWheel)
        {
            GfVec3d camFront = (_at - _eye).GetNormalized();
            _eye += camFront * scrollWheel / 10.f;

            UpdateActiveCam();
        }

        void SetFreeCam() { _activeCam = UsdPrim(); }

        void SetActiveCam(UsdPrim cam)
        {
            _activeCam = cam;
            ReadFromActiveCam();
        }

        void ReadFromActiveCam()
        {
            if (!_activeCam.IsValid()) return;

            UsdGeomCamera geomCam(_activeCam);
            GfCamera gfCam = geomCam.GetCamera(UsdTimeCode::Default());
            GfFrustum frustum = gfCam.GetFrustum();
            _eye = frustum.GetPosition();
            _at = frustum.ComputeLookAtPoint();
        }

        GfMatrix4d getCurViewMatrix()
        {
            return GfMatrix4d().SetLookAt(_eye, _at, _up);
        }

        void UpdateActiveCam()
        {
            if (!_activeCam.IsValid()) return;

            UsdGeomCamera geomCam(_activeCam);
            GfCamera gfCam = geomCam.GetCamera(UsdTimeCode::Default());

            GfFrustum prevFrustum = gfCam.GetFrustum();

            GfMatrix4d view = getCurViewMatrix();
            ;
            GfMatrix4d prevView = prevFrustum.ComputeViewMatrix();
            GfMatrix4d prevProj = prevFrustum.ComputeProjectionMatrix();

            if (view == prevView && _proj == prevProj) return;

            SetTransform(geomCam, view.GetInverse());
        }

        void UpdateCamProjection()
        {
            float fov = FREE_CAM_FOV;
            float near = FREE_CAM_NEAR;
            float far = FREE_CAM_FAR;

            if (_activeCam.IsValid()) {
                UsdGeomCamera geomCam(_activeCam);
                GfCamera gfCam = geomCam.GetCamera(UsdTimeCode::Default());
                fov = gfCam.GetFieldOfView(GfCamera::FOVVertical);
                near = gfCam.GetClippingRange().GetMin();
                far = gfCam.GetClippingRange().GetMax();
            }

            GfFrustum frustum;
            double aspectRatio = GetViewportWidth() / GetViewportHeight();
            frustum.SetPerspective(fov, true, aspectRatio, near, far);
            _proj = frustum.ComputeProjectionMatrix();
        }

        void FocusActiveCamOnPrim(UsdPrim prim)
        {
            if (!prim.IsValid()) return;

            TfTokenVector purposes;
            purposes.push_back(UsdGeomTokens->default_);

            bool useExtentHints = true;
            UsdGeomBBoxCache bboxCache(UsdTimeCode::Default(), purposes,
                                       useExtentHints);
            GfBBox3d bbox = bboxCache.ComputeWorldBound(prim);

            _at = bbox.ComputeCentroid();
            _eye = _at + (_eye - _at).GetNormalized() *
                             bbox.GetBox().GetSize().GetLength() * 2;

            UpdateActiveCam();
        }

        void KeyPressEvent(ImGuiKey key) override
        {
            if (key == ImGuiKey_F) {
                vector<UsdPrim> prims = GetModel()->GetSelection();
                if (prims.size() > 0) FocusActiveCamOnPrim(prims[0]);
            }
            else if (key == ImGuiKey_W) {
                _curOperation = ImGuizmo::TRANSLATE;
                _curMode = ImGuizmo::LOCAL;
            }
            else if (key == ImGuiKey_E) {
                _curOperation = ImGuizmo::ROTATE;
                _curMode = ImGuizmo::LOCAL;
            }
            else if (key == ImGuiKey_R) {
                _curOperation = ImGuizmo::SCALE;
                _curMode = ImGuizmo::LOCAL;
            }
        }

        void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos) override
        {
            ImVec2 deltaMousePos = curPos - prevPos;

            ImGuiIO& io = ImGui::GetIO();
            if (io.MouseWheel) ZoomActiveCam(io.MouseWheel);

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                ImGui::IsKeyDown(ImGuiKey_RightAlt)) {
                OrbitActiveCam(deltaMousePos);
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                ImGui::IsKeyDown(ImGuiKey_RightShift)) {
                PanActiveCam(deltaMousePos);
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
                ImGui::IsKeyDown(ImGuiKey_RightAlt)) {
                ZoomActiveCam(deltaMousePos);
            }
        }

        void MousePressEvent(ImGuiMouseButton_ button,
                             ImVec2 mousePos) override
        {
        }
        void MouseReleaseEvent(ImGuiMouseButton_ button,
                               ImVec2 mousePos) override
        {
            if (button == ImGuiMouseButton_Left) {
                ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                if (fabs(delta.x) + fabs(delta.y) < 0.001f) {
                    UsdPrim prim = FindIntersection(mousePos);

                    // TODO: tmp fix: if parent is instanceable,
                    // modify the selection to the instanceable parent
                    // since instanceable is not handle currently
                    UsdPrim instParent = GetInstanceableParent(prim);
                    if (instParent.IsValid()) prim = instParent;

                    GetModel()->SetSelection({prim});
                }
            }
        }

        UsdPrim FindIntersection(ImVec2 screenPos)
        {
            // create a cam from the current view and proj matrices
            GfCamera gfCam;
            gfCam.SetFromViewAndProjectionMatrix(getCurViewMatrix(), _proj);
            GfFrustum frustum = gfCam.GetFrustum();

            // create a narrowed frustum on the mouse position
            float normalizedScreenPosX = screenPos.x / GetViewportWidth();
            float normalizedScreenPosY = screenPos.y / GetViewportHeight();

            GfVec2d size(1.0 / GetViewportWidth(), 1.0 / GetViewportHeight());

            auto nFrustum = frustum.ComputeNarrowedFrustum(
                GfVec2d(2.0 * normalizedScreenPosX - 1.0,
                        2.0 * (1.0 - normalizedScreenPosY) - 1.0),
                size);

            // check the intersection from the narrowed frustum
            GfVec3d outHitPoint;
            GfVec3d outHitNormal;
            SdfPath outHitPrimPath;
            SdfPath outHitInstancerPath;
            int outHitInstanceIndex;

            UsdStageRefPtr stage = GetModel()->GetStage();
            auto prim = stage->GetPseudoRoot();

            if (_renderer->TestIntersection(
                    nFrustum.ComputeViewMatrix(),
                    nFrustum.ComputeProjectionMatrix(), prim, _renderParams,
                    &outHitPoint, &outHitNormal, &outHitPrimPath,
                    &outHitInstancerPath, &outHitInstanceIndex)) {
                return stage->GetPrimAtPath(outHitPrimPath);
            }
            else return UsdPrim();
        }

        void HoverInEvent() override
        {
            _gizmoWindowFlags |= ImGuiWindowFlags_NoMove;
        }
        void HoverOutEvent() override
        {
            _gizmoWindowFlags &= ~ImGuiWindowFlags_NoMove;
        }
};

PXR_NAMESPACE_CLOSE_SCOPE