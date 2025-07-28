#include "viewport.h"

#include <ImGuiFileDialog.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/imaging/hd/cameraSchema.h>
#include <pxr/imaging/hd/extentSchema.h>
#include <pxr/imaging/hd/xformSchema.h>
#include <pxr/usd/usd/stage.h>

PXR_NAMESPACE_OPEN_SCOPE

Viewport::Viewport(Model* model, const string label) : View(model, label)
{
    _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;
    _isAmbientLightEnabled = true;
    _isDomeLightEnabled = false;
    _isGridEnabled = true;

    _curOperation = ImGuizmo::TRANSLATE;
    _curMode = ImGuizmo::LOCAL;

    _eye = GfVec3d(5, 5, 5);
    _at = GfVec3d(0, 0, 0);
    _up = GfVec3d::YAxis();

    _UpdateActiveCamFromViewport();

    _gridSceneIndex = GridSceneIndex::New();
    GetModel()->AddSceneIndexBase(_gridSceneIndex);

    auto editableSceneIndex = GetModel()->GetEditableSceneIndex();
    _xformSceneIndex = XformFilterSceneIndex::New(editableSceneIndex);
    GetModel()->SetEditableSceneIndex(_xformSceneIndex);
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
        if (ImGui::BeginMenu("Transform")) {
            if (ImGui::MenuItem("Local Translate")) {
                _curOperation = ImGuizmo::TRANSLATE;
                _curMode = ImGuizmo::LOCAL;
            }
            if (ImGui::MenuItem("Local Rotation")) {
                _curOperation = ImGuizmo::ROTATE;
                _curMode = ImGuizmo::LOCAL;
            }
            if (ImGui::MenuItem("Local Scale")) {
                _curOperation = ImGuizmo::SCALE;
                _curMode = ImGuizmo::LOCAL;
            }
            if (ImGui::MenuItem("Global Translate")) {
                _curOperation = ImGuizmo::TRANSLATE;
                _curMode = ImGuizmo::WORLD;
            }
            if (ImGui::MenuItem("Global Rotation")) {
                _curOperation = ImGuizmo::ROTATE;
                _curMode = ImGuizmo::WORLD;
            }
            if (ImGui::MenuItem("Global Scale")) {
                _curOperation = ImGuizmo::SCALE;
                _curMode = ImGuizmo::WORLD;
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Renderer")) {
            // get all possible renderer plugins
            TfTokenVector plugins = _engine->GetRendererPlugins();
            TfToken curPlugin = _engine->GetCurrentRendererPlugin();
            for (auto p : plugins) {
                bool enabled = (p == curPlugin);
                string name = _engine->GetRendererPluginName(p);
                if (ImGui::MenuItem(name.c_str(), NULL, enabled)) {
                    _engine->SetRendererPlugin(p);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Cameras")) {
            bool enabled = (_activeCam.IsEmpty());
            if (ImGui::MenuItem("Free Camera", NULL, &enabled)) {
                _SetFreeCamAsActive();
            }
            for (SdfPath path : GetModel()->GetCameras()) {
                bool enabled = (path == _activeCam);
                if (ImGui::MenuItem(path.GetName().c_str(), NULL, enabled)) {
                    _SetActiveCam(path);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Lights")) {
            bool tmpState = _isAmbientLightEnabled;
            ImGui::MenuItem("Ambient Light", NULL, &_isAmbientLightEnabled);
            if (_isAmbientLightEnabled != tmpState)
                _engine->SetAmbientLightEnabled(_isAmbientLightEnabled);

            tmpState = _isDomeLightEnabled;
            ImGui::MenuItem("Dome Light", NULL, &_isDomeLightEnabled);
            if (_isDomeLightEnabled != tmpState)
                _engine->SetDomeLightEnabled(_isDomeLightEnabled);

            if (ImGui::MenuItem("Load Dome Light Texture ...")) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "DomeLightFile", "Choose File", ".exr,.hdr,.png,.jpg", ".");
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Show")) {
            ImGui::MenuItem("Grid", NULL, &_isGridEnabled);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("DomeLightFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _engine->SetDomeLightTexturePath(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
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
    _gridSceneIndex->Populate(_isGridEnabled);
}

void Viewport::_UpdateHydraRender()
{
    if (!_engine) {
        auto pluginId = Engine::GetDefaultRendererPlugin();
        _engine = new Engine(_sceneIndex, pluginId);
    }

    if(_engine->GetSceneIndex() != _sceneIndex) {
        _engine->SetSceneIndex(_sceneIndex);
    }

    GfMatrix4d view = _getCurViewMatrix();
    float width = _GetViewportWidth();
    float height = _GetViewportHeight();

    // set selection
    SdfPathVector paths;
    for (auto&& prim : GetModel()->GetSelection())
        paths.push_back(prim.GetPrimPath());

    _engine->SetSelection(paths);
    _engine->SetRenderSize(width, height);
    _engine->SetCameraMatrices(view, _proj);

    // do the render
    _engine->Render();

    void* id = _engine->GetRenderBufferData();
    ImGui::Image(id, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
}

void Viewport::_UpdateTransformGuizmo()
{
    SdfPathVector primPaths = GetModel()->GetSelection();
    if (primPaths.size() == 0 || primPaths[0].IsEmpty()) return;

    SdfPath primPath = primPaths[0];

    GfMatrix4d transform = _xformSceneIndex->GetXform(primPath);
    GfMatrix4f transformF(transform);

    GfMatrix4d view = _getCurViewMatrix();

    GfMatrix4f viewF(view);
    GfMatrix4f projF(_proj);

    ImGuizmo::Manipulate(viewF.data(), projF.data(), _curOperation, _curMode,
                         transformF.data());

    if (transformF != GfMatrix4f(transform))
        _xformSceneIndex->SetXform(primPath, GfMatrix4d(transformF));
}

void Viewport::_UpdateCubeGuizmo()
{
    GfMatrix4d view = _getCurViewMatrix();
    GfMatrix4f viewF(view);

    ImGuizmo::ViewManipulate(
        viewF.data(), 8.f,
        ImVec2(GetInnerRect().Max.x - 128, GetInnerRect().Min.y + 18),
        ImVec2(128, 128), IM_COL32_BLACK_TRANS);

    if (viewF != GfMatrix4f(view)) {
        view = GfMatrix4d(viewF);
        GfFrustum frustum;
        frustum.SetPositionAndRotationFromMatrix(view.GetInverse());
        _eye = frustum.GetPosition();
        _at = frustum.ComputeLookAtPoint();

        _UpdateActiveCamFromViewport();
    }
}

void Viewport::_UpdatePluginLabel()
{
    TfToken curPlugin = _engine->GetCurrentRendererPlugin();
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
    GfVec3d camFront = _at - _eye;
    GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
    GfVec3d camUp = GfCross(camRight, camFront).GetNormalized();

    GfVec3d delta =
        camRight * -mouseDeltaPos.x / 100.f + camUp * mouseDeltaPos.y / 100.f;

    _eye += delta;
    _at += delta;

    _UpdateActiveCamFromViewport();
}

void Viewport::_OrbitActiveCam(ImVec2 mouseDeltaPos)
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

    _UpdateActiveCamFromViewport();
}

void Viewport::_ZoomActiveCam(ImVec2 mouseDeltaPos)
{
    GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * mouseDeltaPos.x / 100.f;

    _UpdateActiveCamFromViewport();
}

void Viewport::_ZoomActiveCam(float scrollWheel)
{
    GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * scrollWheel;

    _UpdateActiveCamFromViewport();
}

void Viewport::_SetFreeCamAsActive()
{
    _activeCam = SdfPath();
}

void Viewport::_SetActiveCam(SdfPath primPath)
{
    _activeCam = primPath;
    _UpdateViewportFromActiveCam();
}

void Viewport::_UpdateViewportFromActiveCam()
{
    if (_activeCam.IsEmpty()) return;

    HdSceneIndexPrim prim = _sceneIndex->GetPrim(_activeCam);
    GfCamera gfCam = _ToGfCamera(prim);
    GfFrustum frustum = gfCam.GetFrustum();
    _eye = frustum.GetPosition();
    _at = frustum.ComputeLookAtPoint();
}

GfMatrix4d Viewport::_getCurViewMatrix()
{
    return GfMatrix4d().SetLookAt(_eye, _at, _up);
}

void Viewport::_UpdateActiveCamFromViewport()
{
    if (_activeCam.IsEmpty()) return;

    HdSceneIndexPrim prim = _sceneIndex->GetPrim(_activeCam);
    GfCamera gfCam = _ToGfCamera(prim);

    GfFrustum prevFrustum = gfCam.GetFrustum();

    GfMatrix4d view = _getCurViewMatrix();
    ;
    GfMatrix4d prevView = prevFrustum.ComputeViewMatrix();
    GfMatrix4d prevProj = prevFrustum.ComputeProjectionMatrix();

    if (view == prevView && _proj == prevProj) return;

    _xformSceneIndex->SetXform(_activeCam, view.GetInverse());
}

void Viewport::_UpdateProjection()
{
    float fov = _FREE_CAM_FOV;
    float nearPlane = _FREE_CAM_NEAR;
    float farPlane = _FREE_CAM_FAR;

    if (!_activeCam.IsEmpty()) {
        HdSceneIndexPrim prim = _sceneIndex->GetPrim(_activeCam);
        GfCamera gfCam = _ToGfCamera(prim);
        fov = gfCam.GetFieldOfView(GfCamera::FOVVertical);
        nearPlane = gfCam.GetClippingRange().GetMin();
        farPlane = gfCam.GetClippingRange().GetMax();
    }

    GfFrustum frustum;
    double aspectRatio = _GetViewportWidth() / _GetViewportHeight();
    frustum.SetPerspective(fov, true, aspectRatio, nearPlane, farPlane);
    _proj = frustum.ComputeProjectionMatrix();
}

GfCamera Viewport::_ToGfCamera(HdSceneIndexPrim prim)
{
    GfCamera cam;

    if (prim.primType != HdPrimTypeTokens->camera) return cam;

    HdSampledDataSource::Time time(0);

    HdXformSchema xformSchema = HdXformSchema::GetFromParent(prim.dataSource);

    GfMatrix4d xform =
        xformSchema.GetMatrix()->GetValue(time).Get<GfMatrix4d>();

    HdCameraSchema camSchema = HdCameraSchema::GetFromParent(prim.dataSource);

    TfToken projection =
        camSchema.GetProjection()->GetValue(time).Get<TfToken>();
    float hAperture =
        camSchema.GetHorizontalAperture()->GetValue(time).Get<float>();
    float vAperture =
        camSchema.GetVerticalAperture()->GetValue(time).Get<float>();
    float hApertureOffest =
        camSchema.GetHorizontalApertureOffset()->GetValue(time).Get<float>();
    float vApertureOffest =
        camSchema.GetVerticalApertureOffset()->GetValue(time).Get<float>();
    float focalLength =
        camSchema.GetFocalLength()->GetValue(time).Get<float>();
    GfVec2f clippingRange =
        camSchema.GetClippingRange()->GetValue(time).Get<GfVec2f>();

    cam.SetTransform(xform);
    cam.SetProjection(projection == HdCameraSchemaTokens->orthographic
                          ? GfCamera::Orthographic
                          : GfCamera::Perspective);
    cam.SetHorizontalAperture(hAperture / GfCamera::APERTURE_UNIT);
    cam.SetVerticalAperture(vAperture / GfCamera::APERTURE_UNIT);
    cam.SetHorizontalApertureOffset(hApertureOffest / GfCamera::APERTURE_UNIT);
    cam.SetVerticalApertureOffset(vApertureOffest / GfCamera::APERTURE_UNIT);
    cam.SetFocalLength(focalLength / GfCamera::FOCAL_LENGTH_UNIT);
    cam.SetClippingRange(GfRange1f(clippingRange[0], clippingRange[1]));

    return cam;
}

void Viewport::_FocusOnPrim(SdfPath primPath)
{
    if (primPath.IsEmpty()) return;

    HdSceneIndexPrim prim = _sceneIndex->GetPrim(primPath);

    HdExtentSchema extentSchema =
        HdExtentSchema::GetFromParent(prim.dataSource);
    if (!extentSchema.IsDefined()) {
        TF_WARN("Prim at %s has no extent; skipping focus.",
                primPath.GetAsString().c_str());
        return;
    }

    HdSampledDataSource::Time time(0);
    GfVec3d extentMin = extentSchema.GetMin()->GetValue(time).Get<GfVec3d>();
    GfVec3d extentMax = extentSchema.GetMax()->GetValue(time).Get<GfVec3d>();

    GfRange3d extentRange(extentMin, extentMax);

    _at = extentRange.GetMidpoint();
    _eye = _at + (_eye - _at).GetNormalized() *
                     extentRange.GetSize().GetLength() * 2;

    _UpdateActiveCamFromViewport();
}

void Viewport::_KeyPressEvent(ImGuiKey key)
{
    if (key == ImGuiKey_F) {
        SdfPathVector primPaths = GetModel()->GetSelection();
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
            GfVec2f gfMousePos(mousePos[0], mousePos[1]);
            SdfPath primPath = _engine->FindIntersection(gfMousePos);

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

PXR_NAMESPACE_CLOSE_SCOPE