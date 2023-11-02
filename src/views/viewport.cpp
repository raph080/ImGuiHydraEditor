#include "viewport.h"

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>

Viewport::Viewport(Model* model, const string label) : View(model, label)
{
    _drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(100));
    _drawTarget->Bind();
    _drawTarget->AddAttachment(pxr::HdAovTokens->color, GL_RGBA, GL_FLOAT,
                               GL_RGBA);
    _drawTarget->AddAttachment(pxr::HdAovTokens->depth, GL_DEPTH_COMPONENT,
                               GL_FLOAT, GL_DEPTH_COMPONENT);
    _drawTarget->Unbind();

    _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;
    _isAmbientLightEnabled = true;
    _isDomeLightEnabled = false;
    _isGridEnabled = true;

    _curOperation = ImGuizmo::TRANSLATE;
    _curMode = ImGuizmo::LOCAL;

    _eye = pxr::GfVec3d(5, 5, 5);
    _at = pxr::GfVec3d(0, 0, 0);
    _up = GetModel()->GetUpAxis();

    UpdateActiveCamFromViewport();

    _renderer = new pxr::UsdImagingGLEngine();
    _curPlugin = _renderer->GetCurrentRendererId();
};
Viewport::~Viewport()
{
    _drawTarget = pxr::GlfDrawTargetRefPtr();
    delete _renderer;
}

const string Viewport::GetViewType()
{
    return VIEW_TYPE;
};

ImGuiWindowFlags Viewport::GetGizmoWindowFlags()
{
    return _gizmoWindowFlags;
};

void Viewport::ModelChangedEvent()
{
    delete _renderer;
    _renderer = new pxr::UsdImagingGLEngine();
    _renderer->SetRendererPlugin(_curPlugin);
    _up = GetModel()->GetUpAxis();
};

float Viewport::GetViewportWidth()
{
    return GetInnerRect().GetWidth();
}
float Viewport::GetViewportHeight()
{
    return GetInnerRect().GetHeight();
}

void Viewport::Draw()
{
    DrawMenuBar();

    if (GetViewportWidth() <= 0 || GetViewportHeight() <= 0) return;

    ImGui::BeginChild("GameRender");

    ConfigureImGuizmo();

    // read from active cam in case it is modify by another view
    if (!ImGui::IsWindowFocused()) UpdateViewportFromActiveCam();

    UpdateProjection();
    UpdateGrid();
    UpdateUsdRender();
    UpdateTransformGuizmo();
    UpdateCubeGuizmo();
    UpdatePluginLabel();

    ImGui::EndChild();
};

void Viewport::DrawMenuBar()
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
            pxr::TfTokenVector plugins = _renderer->GetRendererPlugins();
            for (auto p : plugins) {
                bool enabled = (p == _curPlugin);
                string name =
                    pxr::UsdImagingGLEngine::GetRendererDisplayName(p);
                if (ImGui::MenuItem(name.c_str(), NULL, enabled)) {
                    _renderer->SetRendererPlugin(p);
                    _curPlugin = p;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("cameras")) {
            bool enabled = (!_activeCam.IsValid());
            if (ImGui::MenuItem("free camera", NULL, &enabled)) {
                SetFreeCamAsActive();
            }
            for (pxr::UsdPrim cam : GetModel()->GetCameras()) {
                bool enabled = (cam == _activeCam);
                if (ImGui::MenuItem(cam.GetName().GetText(), NULL, enabled)) {
                    SetActiveCam(cam);
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

void Viewport::ConfigureImGuizmo()
{
    ImGuizmo::BeginFrame();

    // convert last label char to ID
    string label = GetViewLabel();
    ImGuizmo::SetID(int(label[label.size() - 1]));

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(GetInnerRect().Min.x, GetInnerRect().Min.y,
                      GetViewportWidth(), GetViewportHeight());
}

void Viewport::UpdateGrid()
{
    if (!_isGridEnabled) return;

    pxr::GfMatrix4f viewF(getCurViewMatrix());
    pxr::GfMatrix4f projF(_proj);
    pxr::GfMatrix4f identity(1);

    ImGuizmo::DrawGrid(viewF.data(), projF.data(), identity.data(), 10);
}

void Viewport::UpdateUsdRender()
{
    pxr::UsdStageRefPtr stage = GetModel()->GetStage();
    pxr::GfMatrix4d view = getCurViewMatrix();
    float width = GetViewportWidth();
    float height = GetViewportHeight();

    _renderer->SetRendererAov(pxr::HdAovTokens->color);
    _renderer->SetRenderBufferSize(pxr::GfVec2i(width, height));
    _renderer->SetCameraState(view, _proj);
    _renderer->SetOverrideWindowPolicy(
        {false,
         pxr::CameraUtilConformWindowPolicy::CameraUtilMatchVertically});

    // set selection
    pxr::SdfPathVector paths;
    for (auto&& prim : GetModel()->GetSelection())
        paths.push_back(prim.GetPrimPath());

    _renderer->SetSelected(paths);

    // set the framing
    auto displayWindow =
        pxr::GfRange2f(pxr::GfVec2f(0.0, 0.0), pxr::GfVec2f(width, height));
    // -1 to prevent error "dataWindow is larger than render buffer"
    auto dataWindow =
        pxr::GfRect2i(pxr::GfVec2i(0, 0), pxr::GfVec2i(width - 1, height - 1));
    _renderer->SetFraming(pxr::CameraUtilFraming(displayWindow, dataWindow));

    // set lighting state
    auto sceneAmbient = pxr::GfVec4f(0.01, 0.01, 0.01, 1.0);
    auto material = pxr::GlfSimpleMaterial();
    auto lights = pxr::GlfSimpleLightVector();

    if (_isAmbientLightEnabled) {
        pxr::GlfSimpleLight l;
        l.SetAmbient(pxr::GfVec4f(0, 0, 0, 0));
        l.SetPosition(pxr::GfVec4f(_eye[0], _eye[1], _eye[2], 1));
        lights.push_back(l);
    }
    if (_isDomeLightEnabled) {
        pxr::GlfSimpleLight l;
        l.SetIsDomeLight(true);
        if (GetModel()->GetUpAxis() == pxr::GfVec3d::ZAxis()) {
            pxr::GfMatrix4d rotMat = pxr::GfMatrix4d().SetRotate(
                pxr::GfRotation(pxr::GfVec3d::XAxis(), 90));
            l.SetTransform(rotMat);
        }
        lights.push_back(l);
    }

    material.SetAmbient(pxr::GfVec4f(0.2, 0.2, 0.2, 1));
    material.SetSpecular(pxr::GfVec4f(0.1, 0.1, 0.1, 1));
    material.SetShininess(32.0);

    _renderer->SetLightingState(lights, material, sceneAmbient);

    // set render params
    _renderParams.frame = pxr::UsdTimeCode::Default();
    _renderParams.complexity = 1.0;
    _renderParams.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
    _renderParams.showGuides = false;
    _renderParams.showProxy = true;
    _renderParams.showRender = false;
    _renderParams.forceRefresh = true;
    _renderParams.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;
    _renderParams.gammaCorrectColors = false;
    _renderParams.enableIdRender = false;
    _renderParams.enableSampleAlphaToCoverage = true;
    _renderParams.highlight = true;
    _renderParams.enableSceneMaterials = true;
    _renderParams.enableSceneLights = true;
    _renderParams.clearColor = pxr::GfVec4f(0.7, 0.7, 0.7, 0);

    // do the actual render in the draw target
    _drawTarget->Bind();
    _drawTarget->SetSize(pxr::GfVec2i(width, height));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // do the render
    _renderer->Render(stage->GetPseudoRoot(), _renderParams);

    _drawTarget->Unbind();

    // create an imgui image with the drawtarget color data
    GLuint id = _drawTarget->GetAttachment(pxr::HdAovTokens->color)
                    ->GetGlTextureName();
    ImGui::Image((ImTextureID)id, ImVec2(width, height), ImVec2(0, 1),
                 ImVec2(1, 0));
}

void Viewport::UpdateTransformGuizmo()
{
    vector<pxr::UsdPrim> prims = GetModel()->GetSelection();
    if (prims.size() == 0 || !prims[0].IsValid()) return;

    pxr::UsdGeomGprim geom(prims[0]);

    pxr::GfMatrix4d transform = GetTransformMatrix(geom);
    pxr::GfMatrix4f transformF(transform);

    pxr::GfMatrix4d view = getCurViewMatrix();

    pxr::GfMatrix4f viewF(view);
    pxr::GfMatrix4f projF(_proj);

    ImGuizmo::Manipulate(viewF.data(), projF.data(), _curOperation, _curMode,
                         transformF.data());

    if (transformF != pxr::GfMatrix4f(transform))
        SetTransformMatrix(geom, pxr::GfMatrix4d(transformF));
}

void Viewport::UpdateCubeGuizmo()
{
    pxr::GfMatrix4d view = getCurViewMatrix();
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

        UpdateActiveCamFromViewport();
    }
}

void Viewport::UpdatePluginLabel()
{
    string pluginText =
        pxr::UsdImagingGLEngine::GetRendererDisplayName(_curPlugin);
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

void Viewport::PanActiveCam(ImVec2 mouseDeltaPos)
{
    pxr::GfVec3d camFront = _at - _eye;
    pxr::GfVec3d camRight = GfCross(camFront, _up).GetNormalized();
    pxr::GfVec3d camUp = GfCross(camRight, camFront).GetNormalized();

    pxr::GfVec3d delta =
        camRight * -mouseDeltaPos.x / 100.f + camUp * mouseDeltaPos.y / 100.f;

    _eye += delta;
    _at += delta;

    UpdateActiveCamFromViewport();
}

void Viewport::OrbitActiveCam(ImVec2 mouseDeltaPos)
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

    UpdateActiveCamFromViewport();
}

void Viewport::ZoomActiveCam(ImVec2 mouseDeltaPos)
{
    pxr::GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * mouseDeltaPos.x / 100.f;

    UpdateActiveCamFromViewport();
}

void Viewport::ZoomActiveCam(float scrollWheel)
{
    pxr::GfVec3d camFront = (_at - _eye).GetNormalized();
    _eye += camFront * scrollWheel / 10.f;

    UpdateActiveCamFromViewport();
}

void Viewport::SetFreeCamAsActive()
{
    _activeCam = pxr::UsdPrim();
}

void Viewport::SetActiveCam(pxr::UsdPrim cam)
{
    _activeCam = cam;
    UpdateViewportFromActiveCam();
}

void Viewport::UpdateViewportFromActiveCam()
{
    if (!_activeCam.IsValid()) return;

    pxr::UsdGeomCamera geomCam(_activeCam);
    pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());
    pxr::GfFrustum frustum = gfCam.GetFrustum();
    _eye = frustum.GetPosition();
    _at = frustum.ComputeLookAtPoint();
}

pxr::GfMatrix4d Viewport::getCurViewMatrix()
{
    return pxr::GfMatrix4d().SetLookAt(_eye, _at, _up);
}

void Viewport::UpdateActiveCamFromViewport()
{
    if (!_activeCam.IsValid()) return;

    pxr::UsdGeomCamera geomCam(_activeCam);
    pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());

    pxr::GfFrustum prevFrustum = gfCam.GetFrustum();

    pxr::GfMatrix4d view = getCurViewMatrix();
    ;
    pxr::GfMatrix4d prevView = prevFrustum.ComputeViewMatrix();
    pxr::GfMatrix4d prevProj = prevFrustum.ComputeProjectionMatrix();

    if (view == prevView && _proj == prevProj) return;

    SetTransformMatrix(geomCam, view.GetInverse());
}

void Viewport::UpdateProjection()
{
    float fov = _FREE_CAM_FOV;
    float nearPlane = _FREE_CAM_NEAR;
    float farPlane = _FREE_CAM_FAR;

    if (_activeCam.IsValid()) {
        pxr::UsdGeomCamera geomCam(_activeCam);
        pxr::GfCamera gfCam = geomCam.GetCamera(pxr::UsdTimeCode::Default());
        fov = gfCam.GetFieldOfView(pxr::GfCamera::FOVVertical);
        nearPlane = gfCam.GetClippingRange().GetMin();
        farPlane = gfCam.GetClippingRange().GetMax();
    }

    pxr::GfFrustum frustum;
    double aspectRatio = GetViewportWidth() / GetViewportHeight();
    frustum.SetPerspective(fov, true, aspectRatio, nearPlane, farPlane);
    _proj = frustum.ComputeProjectionMatrix();
}

void Viewport::FocusOnPrim(pxr::UsdPrim prim)
{
    if (!prim.IsValid()) return;

    pxr::TfTokenVector purposes;
    purposes.push_back(pxr::UsdGeomTokens->default_);

    bool useExtentHints = true;
    pxr::UsdGeomBBoxCache bboxCache(pxr::UsdTimeCode::Default(), purposes,
                                    useExtentHints);
    pxr::GfBBox3d bbox = bboxCache.ComputeWorldBound(prim);

    _at = bbox.ComputeCentroid();
    _eye = _at + (_eye - _at).GetNormalized() *
                     bbox.GetBox().GetSize().GetLength() * 2;

    UpdateActiveCamFromViewport();
}

void Viewport::KeyPressEvent(ImGuiKey key)
{
    if (key == ImGuiKey_F) {
        vector<pxr::UsdPrim> prims = GetModel()->GetSelection();
        if (prims.size() > 0) FocusOnPrim(prims[0]);
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

void Viewport::MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos)
{
    ImVec2 deltaMousePos = curPos - prevPos;

    ImGuiIO& io = ImGui::GetIO();
    if (io.MouseWheel) ZoomActiveCam(io.MouseWheel);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
         ImGui::IsKeyDown(ImGuiKey_RightAlt))) {
        OrbitActiveCam(deltaMousePos);
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftShift) ||
         ImGui::IsKeyDown(ImGuiKey_RightShift))) {
        PanActiveCam(deltaMousePos);
    }
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftAlt) ||
         ImGui::IsKeyDown(ImGuiKey_RightAlt))) {
        ZoomActiveCam(deltaMousePos);
    }
}

void Viewport::MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 mousePos)
{
    if (button == ImGuiMouseButton_Left) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        if (fabs(delta.x) + fabs(delta.y) < 0.001f) {
            pxr::UsdPrim prim = FindIntersection(mousePos);

            // TODO: tmp fix: if parent is instanceable,
            // modify the selection to the instanceable parent
            // since instanceable is not handle currently
            pxr::UsdPrim instParent = GetInstanceableParent(prim);
            if (instParent.IsValid()) prim = instParent;

            GetModel()->SetSelection({prim});
        }
    }
}

pxr::UsdPrim Viewport::FindIntersection(ImVec2 mousePos)
{
    // create a cam from the current view and proj matrices
    pxr::GfCamera gfCam;
    gfCam.SetFromViewAndProjectionMatrix(getCurViewMatrix(), _proj);
    pxr::GfFrustum frustum = gfCam.GetFrustum();

    // create a narrowed frustum on the mouse position
    float normalizedMousePosX = mousePos.x / GetViewportWidth();
    float normalizedMousePosY = mousePos.y / GetViewportHeight();

    pxr::GfVec2d size(1.0 / GetViewportWidth(), 1.0 / GetViewportHeight());

    auto nFrustum = frustum.ComputeNarrowedFrustum(
        pxr::GfVec2d(2.0 * normalizedMousePosX - 1.0,
                     2.0 * (1.0 - normalizedMousePosY) - 1.0),
        size);

    // check the intersection from the narrowed frustum
    pxr::GfVec3d outHitPoint;
    pxr::GfVec3d outHitNormal;
    pxr::SdfPath outHitPrimPath;
    pxr::SdfPath outHitInstancerPath;
    int outHitInstanceIndex;

    pxr::UsdStageRefPtr stage = GetModel()->GetStage();
    auto prim = stage->GetPseudoRoot();

    if (_renderer->TestIntersection(
            nFrustum.ComputeViewMatrix(), nFrustum.ComputeProjectionMatrix(),
            prim, _renderParams, &outHitPoint, &outHitNormal, &outHitPrimPath,
            &outHitInstancerPath, &outHitInstanceIndex)) {
        return stage->GetPrimAtPath(outHitPrimPath);
    }
    else return pxr::UsdPrim();
}

void Viewport::HoverInEvent()
{
    _gizmoWindowFlags |= ImGuiWindowFlags_NoMove;
}
void Viewport::HoverOutEvent()
{
    _gizmoWindowFlags &= ~ImGuiWindowFlags_NoMove;
}