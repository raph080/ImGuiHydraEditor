#include "viewport.h"

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>

Viewport::Viewport(Model* model, const string label) : View(model, label)
{
    _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;
    _isAmbientLightEnabled = true;
    _isDomeLightEnabled = false;
    _isGridEnabled = true;

    _curOperation = ImGuizmo::TRANSLATE;
    _curMode = ImGuizmo::LOCAL;

    _eye = pxr::GfVec3d(5, 5, 5);
    _at = pxr::GfVec3d(0, 0, 0);
    _up = GetModel()->GetUpAxis();

    _UpdateActiveCamFromViewport();

    pxr::TfToken plugin = Engine::GetDefaultRendererPlugin();
    pxr::UsdStageRefPtr stage = GetModel()->GetStage();
    _engine = new Engine(stage, plugin);
};

Viewport::~Viewport()
{
    delete _engine;
}

const string Viewport::GetViewType()
{
    return VIEW_TYPE;
};

ImGuiWindowFlags Viewport::_GetGizmoWindowFlags()
{
    return _gizmoWindowFlags;
};

void Viewport::ModelChangedEvent()
{
    pxr::TfToken plugin = _engine->GetCurrentRendererPlugin();
    delete _engine;

    pxr::UsdStageRefPtr stage = GetModel()->GetStage();
    _engine = new Engine(stage, plugin);

    _up = GetModel()->GetUpAxis();
};

float Viewport::_GetViewportWidth()
{
    return GetInnerRect().GetWidth();
}
float Viewport::_GetViewportHeight()
{
    return GetInnerRect().GetHeight();
}

void Viewport::_Draw()
{
    _DrawMenuBar();

    if (_GetViewportWidth() <= 0 || _GetViewportHeight() <= 0) return;

    ImGui::BeginChild("GameRender");

    _ConfigureImGuizmo();

    // read from active cam in case it is modify by another view
    if (!ImGui::IsWindowFocused()) _UpdateViewportFromActiveCam();

    _UpdateProjection();
    _UpdateGrid();
    _UpdateHydraRender();
    _UpdateTransformGuizmo();
    _UpdateCubeGuizmo();
    _UpdatePluginLabel();

    ImGui::EndChild();
};

void Viewport::_DrawMenuBar()
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
            pxr::TfTokenVector plugins = _engine->GetRendererPlugins();
            pxr::TfToken curPlugin = _engine->GetCurrentRendererPlugin();
            for (auto p : plugins) {
                bool enabled = (p == curPlugin);
                string name = _engine->GetRendererPluginName(p);
                if (ImGui::MenuItem(name.c_str(), NULL, enabled)) {
                    delete _engine;
                    _engine = new Engine(GetModel()->GetStage(), p);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("cameras")) {
            bool enabled = (_activeCam.IsEmpty());
            if (ImGui::MenuItem("free camera", NULL, &enabled)) {
                _SetFreeCamAsActive();
            }
            for (pxr::SdfPath path : GetModel()->GetCameras()) {
                bool enabled = (path == _activeCam);
                if (ImGui::MenuItem(path.GetName().c_str(), NULL, enabled)) {
                    _SetActiveCam(path);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("lights")) {
            ImGui::MenuItem("ambient light", NULL, &_isAmbientLightEnabled);
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

void Viewport::_ConfigureImGuizmo()
{
    ImGuizmo::BeginFrame();

    // convert last label char to ID
    string label = GetViewLabel();
    ImGuizmo::SetID(int(label[label.size() - 1]));

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(GetInnerRect().Min.x, GetInnerRect().Min.y,
                      _GetViewportWidth(), _GetViewportHeight());
}

void Viewport::_UpdateGrid()
{
    if (!_isGridEnabled) return;

    pxr::GfMatrix4f viewF(_getCurViewMatrix());
    pxr::GfMatrix4f projF(_proj);
    pxr::GfMatrix4f identity(1);

    ImGuizmo::DrawGrid(viewF.data(), projF.data(), identity.data(), 10);
}

void Viewport::_UpdateHydraRender()
{
    pxr::UsdStageRefPtr stage = GetModel()->GetStage();
    pxr::GfMatrix4d view = _getCurViewMatrix();
    float width = _GetViewportWidth();
    float height = _GetViewportHeight();

    // set selection
    pxr::SdfPathVector paths;
    for (auto&& prim : GetModel()->GetSelection())
        paths.push_back(prim.GetPrimPath());

    _engine->SetSelection(paths);
    _engine->SetRenderSize(width, height);
    _engine->SetCameraMatrices(view, _proj);
    _engine->Prepare();

    // do the render
    _engine->Render();

    // create an imgui image with the drawtarget color data
    void* id = _engine->GetRenderBufferData();
    ImGui::Image(id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
}

void Viewport::_UpdateTransformGuizmo()
{
    pxr::SdfPathVector primPath = GetModel()->GetSelection();
    if (primPath.size() == 0 || primPath[0].IsEmpty()) return;

    pxr::UsdPrim prim = GetModel()->GetPrim(primPath[0]);
    pxr::UsdGeomGprim geom(prim);

    pxr::GfMatrix4d transform = GetTransformMatrix(geom);
    pxr::GfMatrix4f transformF(transform);

    pxr::GfMatrix4d view = _getCurViewMatrix();

    pxr::GfMatrix4f viewF(view);
    pxr::GfMatrix4f projF(_proj);

    ImGuizmo::Manipulate(viewF.data(), projF.data(), _curOperation, _curMode,
                         transformF.data());

    if (transformF != pxr::GfMatrix4f(transform))
        SetTransformMatrix(geom, pxr::GfMatrix4d(transformF));
}

void Viewport::_UpdateCubeGuizmo()
{
    pxr::GfMatrix4d view = _getCurViewMatrix();
    pxr::GfMatrix4f viewF(view);

    ImGuizmo::ViewManipulate(
        viewF.data(), 8.f,
        ImVec2(GetInnerRect().Max.x - 128, GetInnerRect().Min.y + 18),
        ImVec2(128, 128), IM_COL32_BLACK_TRANS);

    if (viewF != pxr::GfMatrix4f(view)) {
        view = pxr::GfMatrix4d(viewF);
        pxr::GfFrustum frustum;
        frustum.SetPositionAndRotationFromMatrix(view.GetInverse());
        _eye = frustum.GetPosition();
        _at = frustum.ComputeLookAtPoint();

        _UpdateActiveCamFromViewport();
    }
}

void Viewport::_UpdatePluginLabel()
{
    pxr::TfToken curPlugin = _engine->GetCurrentRendererPlugin();
    string pluginText = _engine->GetRendererPluginName(curPlugin);
    string text = pluginText;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
    float margin = 6;
    float xPos = (GetInnerRect().Max.x - 64 - textSize.x / 2);
    float yPos = GetInnerRect().Min.y + margin * 2;
    // draw background color
    draw_list->AddRectFilled(
        ImVec2(xPos - margin, yPos - margin),
        ImVec2(xPos + textSize.x + margin, yPos + textSize.y + margin),
        ImColor(.0f, .0f, .0f, .2f), margin);
    // draw text
    draw_list->AddText(ImVec2(xPos, yPos), ImColor(1.f, 1.f, 1.f),
                       text.c_str());
}

void Viewport::_PanActiveCam(ImVec2 mouseDeltaPos)
{
    pxr::GfVec3d camFront = _at - _eye;
    pxr::GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
    pxr::GfVec3d camUp = GfCross(camRight, camFront).GetNormalized();

    pxr::GfVec3d delta =
        camRight * -mouseDeltaPos.x / 100.f + camUp * mouseDeltaPos.y / 100.f;

    _eye += delta;
    _at += delta;

    _UpdateActiveCamFromViewport();
}

void Viewport::_OrbitActiveCam(ImVec2 mouseDeltaPos)
{
    pxr::GfRotation rot(_up, mouseDeltaPos.x / 2);
    pxr::GfMatrix4d rotMatrix = pxr::GfMatrix4d(1).SetRotate(rot);
    pxr::GfVec3d e = _eye - _at;
    pxr::GfVec4d vec4 = rotMatrix * pxr::GfVec4d(e[0], e[1], e[2], 1.f);
    _eye = _at + pxr::GfVec3d(vec4[0], vec4[1], vec4[2]);

    pxr::GfVec3d camFront = _at - _eye;
    pxr::GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
    rot = pxr::GfRotation(camRight, mouseDeltaPos.y / 2);
    rotMatrix = pxr::GfMatrix4d(1).SetRotate(rot);
    e = _eye - _at;
    vec4 = rotMatrix * pxr::GfVec4d(e[0], e[1], e[2], 1.f);
    _eye = _at + pxr::GfVec3d(vec4[0], vec4[1], vec4[2]);

    _UpdateActiveCamFromViewport();
}

void Viewport::_ZoomActiveCam(ImVec2 mouseDeltaPos)
{
    pxr::GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * mouseDeltaPos.x / 100.f;

    _UpdateActiveCamFromViewport();
}

void Viewport::_ZoomActiveCam(float scrollWheel)
{
    pxr::GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * scrollWheel / 10.f;

    _UpdateActiveCamFromViewport();
}

void Viewport::_SetFreeCamAsActive()
{
    _activeCam = pxr::SdfPath();
}

void Viewport::_SetActiveCam(pxr::SdfPath primPath)
{
    _activeCam = primPath;
    _UpdateViewportFromActiveCam();
}

void Viewport::_UpdateViewportFromActiveCam()
{
    if (_activeCam.IsEmpty()) return;

    pxr::UsdPrim primCam = GetModel()->GetPrim(_activeCam);
    pxr::UsdGeomCamera geomCam(primCam);
    pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());
    pxr::GfFrustum frustum = gfCam.GetFrustum();
    _eye = frustum.GetPosition();
    _at = frustum.ComputeLookAtPoint();
}

pxr::GfMatrix4d Viewport::_getCurViewMatrix()
{
    return pxr::GfMatrix4d().SetLookAt(_eye, _at, _up);
}

void Viewport::_UpdateActiveCamFromViewport()
{
    if (_activeCam.IsEmpty()) return;

    pxr::UsdPrim primCam = GetModel()->GetPrim(_activeCam);
    pxr::UsdGeomCamera geomCam(primCam);
    pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());

    pxr::GfFrustum prevFrustum = gfCam.GetFrustum();

    pxr::GfMatrix4d view = _getCurViewMatrix();
    ;
    pxr::GfMatrix4d prevView = prevFrustum.ComputeViewMatrix();
    pxr::GfMatrix4d prevProj = prevFrustum.ComputeProjectionMatrix();

    if (view == prevView && _proj == prevProj) return;

    SetTransformMatrix(geomCam, view.GetInverse());
}

void Viewport::_UpdateProjection()
{
    float fov = _FREE_CAM_FOV;
    float nearPlane = _FREE_CAM_NEAR;
    float farPlane = _FREE_CAM_FAR;

    if (!_activeCam.IsEmpty()) {
        pxr::UsdPrim primCam = GetModel()->GetPrim(_activeCam);
        pxr::UsdGeomCamera geomCam(primCam);
        pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());
        fov = gfCam.GetFieldOfView(pxr::GfCamera::FOVVertical);
        nearPlane = gfCam.GetClippingRange().GetMin();
        farPlane = gfCam.GetClippingRange().GetMax();
    }

    pxr::GfFrustum frustum;
    double aspectRatio = _GetViewportWidth() / _GetViewportHeight();
    frustum.SetPerspective(fov, true, aspectRatio, nearPlane, farPlane);
    _proj = frustum.ComputeProjectionMatrix();
}

void Viewport::_FocusOnPrim(pxr::SdfPath primPath)
{
    if (primPath.IsEmpty()) return;

    pxr::TfTokenVector purposes;
    purposes.push_back(pxr::UsdGeomTokens->default_);

    bool useExtentHints = true;
    pxr::UsdGeomBBoxCache bboxCache(pxr::UsdTimeCode::Default(), purposes,
                                    useExtentHints);
    pxr::UsdPrim prim = GetModel()->GetPrim(primPath);
    pxr::GfBBox3d bbox = bboxCache.ComputeWorldBound(prim);

    _at = bbox.ComputeCentroid();
    _eye = _at + (_eye - _at).GetNormalized() *
                     bbox.GetBox().GetSize().GetLength() * 2;

    _UpdateActiveCamFromViewport();
}

void Viewport::_KeyPressEvent(ImGuiKey key)
{
    if (key == ImGuiKey_F) {
        pxr::SdfPathVector primPaths = GetModel()->GetSelection();
        if (primPaths.size() > 0) _FocusOnPrim(primPaths[0]);
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

void Viewport::_MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos)
{
    ImVec2 deltaMousePos = curPos - prevPos;

    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseWheel) _ZoomActiveCam(io.MouseWheel);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
         ImGui::IsKeyDown(ImGuiKey_RightAlt))) {
        _OrbitActiveCam(deltaMousePos);
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
         ImGui::IsKeyDown(ImGuiKey_RightShift))) {
        _PanActiveCam(deltaMousePos);
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
         ImGui::IsKeyDown(ImGuiKey_RightAlt))) {
        _ZoomActiveCam(deltaMousePos);
    }
}

void Viewport::_MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 mousePos)
{
    if (button == ImGuiMouseButton_Left) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        if (fabs(delta.x) + fabs(delta.y) < 0.001f) {
            pxr::GfVec2f gfMousePos(mousePos[0], mousePos[1]);
            pxr::SdfPath primPath = _engine->FindIntersection(gfMousePos);

            if (primPath.IsEmpty()) GetModel()->SetSelection({});
            else GetModel()->SetSelection({primPath});
        }
    }
}

void Viewport::_HoverInEvent()
{
    _gizmoWindowFlags |= ImGuiWindowFlags_NoMove;
}
void Viewport::_HoverOutEvent()
{
    _gizmoWindowFlags &= ~ImGuiWindowFlags_NoMove;
}