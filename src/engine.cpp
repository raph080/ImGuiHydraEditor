#include "engine.h"

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>

Engine::Engine(pxr::UsdStageRefPtr stage, pxr::TfToken plugin)
    : _stage(stage),
      _curRendererPlugin(plugin),
      _hgi(pxr::Hgi::CreatePlatformDefaultHgi()),
      _hgiDriver{pxr::HgiTokens->renderDriver, pxr::VtValue(_hgi.get())},
      _engine(),
      _renderDelegate(nullptr),
      _renderIndex(nullptr),
      _taskController(nullptr),
      _taskControllerId("/defaultTaskController")
{
    _width = 512;
    _height = 512;

    Initialize();
}

Engine::~Engine()
{
    _drawTarget = pxr::GlfDrawTargetRefPtr();

    // Destroy objects in opposite order of construction.
    delete _taskController;

    if (_renderIndex && _sceneIndex) {
        _renderIndex->RemoveSceneIndex(_sceneIndex);
        _stageSceneIndex = nullptr;
        _sceneIndex = nullptr;
    }

    delete _renderIndex;
    _renderDelegate = nullptr;
}

void Engine::Initialize()
{
    // init draw target
    _drawTarget = pxr::GlfDrawTarget::New(pxr::GfVec2i(_width, _height));
    _drawTarget->Bind();
    _drawTarget->AddAttachment(pxr::HdAovTokens->color, GL_RGBA, GL_FLOAT,
                               GL_RGBA);
    _drawTarget->AddAttachment(pxr::HdAovTokens->depth, GL_DEPTH_COMPONENT,
                               GL_FLOAT, GL_DEPTH_COMPONENT);
    _drawTarget->Unbind();

    // init render delegate
    _renderDelegate = GetRenderDelegateFromPlugin(_curRendererPlugin);

    // init render index
    _renderIndex =
        pxr::HdRenderIndex::New(_renderDelegate.Get(), {&_hgiDriver});

    pxr::UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    const pxr::UsdImagingSceneIndices sceneIndices =
        UsdImagingCreateSceneIndices(info);

    _stageSceneIndex = sceneIndices.stageSceneIndex;
    _sceneIndex = sceneIndices.finalSceneIndex;

    _renderIndex->InsertSceneIndex(_sceneIndex, _taskControllerId);
    _stageSceneIndex->SetStage(_stage);

    // init task controller

    _taskController =
        new pxr::HdxTaskController(_renderIndex, _taskControllerId);

    // init render paramss
    pxr::HdxRenderTaskParams params;
    params.viewport = pxr::GfVec4f(0, 0, _width, _height);
    params.enableLighting = true;
    _taskController->SetRenderParams(params);

    // init collection
    _collection = pxr::HdRprimCollection(
        pxr::HdTokens->geometry,
        pxr::HdReprSelector(pxr::HdReprTokens->smoothHull));
    _taskController->SetCollection(_collection);

    // init render tags
    _taskController->SetRenderTags(pxr::TfTokenVector());

    // init AOVs
    pxr::TfTokenVector _aovOutputs{pxr::HdAovTokens->color};
    _taskController->SetRenderOutputs(_aovOutputs);

    pxr::GfVec4f clearColor = pxr::GfVec4f(.1f, .1f, .1f, 1.0f);
    pxr::HdAovDescriptor colorAovDesc =
        _taskController->GetRenderOutputSettings(pxr::HdAovTokens->color);
    if (colorAovDesc.format != pxr::HdFormatInvalid) {
        colorAovDesc.clearValue = pxr::VtValue(clearColor);
        _taskController->SetRenderOutputSettings(pxr::HdAovTokens->color,
                                                 colorAovDesc);
    }

    // init selection
    pxr::GfVec4f selectionColor = pxr::GfVec4f(1.f, 1.f, 0.f, .5f);
    _selTracker = std::make_shared<pxr::HdxSelectionTracker>();

    _taskController->SetEnableSelection(true);
    _taskController->SetSelectionColor(selectionColor);

    pxr::VtValue selectionValue(_selTracker);
    _engine.SetTaskContextData(pxr::HdxTokens->selectionState, selectionValue);

    _taskController->SetOverrideWindowPolicy(pxr::CameraUtilFit);
}

pxr::TfTokenVector Engine::GetRendererPlugins()
{
    pxr::HfPluginDescVector pluginDescriptors;
    pxr::HdRendererPluginRegistry::GetInstance().GetPluginDescs(
        &pluginDescriptors);

    pxr::TfTokenVector plugins;
    for (size_t i = 0; i < pluginDescriptors.size(); ++i) {
        plugins.push_back(pluginDescriptors[i].id);
    }
    return plugins;
}

pxr::TfToken Engine::GetDefaultRendererPlugin()
{
    pxr::HdRendererPluginRegistry& registry =
        pxr::HdRendererPluginRegistry::GetInstance();
    return registry.GetDefaultPluginId(true);
}

pxr::TfToken Engine::GetCurrentRendererPlugin()
{
    return _curRendererPlugin;
}

pxr::HdPluginRenderDelegateUniqueHandle Engine::GetRenderDelegateFromPlugin(
    pxr::TfToken plugin)
{
    pxr::HdRendererPluginRegistry& registry =
        pxr::HdRendererPluginRegistry::GetInstance();

    pxr::TfToken resolvedId = registry.GetDefaultPluginId(true);

    return registry.CreateRenderDelegate(plugin);
}

string Engine::GetRendererPluginName(pxr::TfToken plugin)
{
    pxr::HfPluginDesc pluginDescriptor;
    bool foundPlugin =
        pxr::HdRendererPluginRegistry::GetInstance().GetPluginDesc(
            plugin, &pluginDescriptor);

    if (!foundPlugin) { return std::string(); }

    // TODO: fix that will be eventually delegate to Hgi
#if defined(__APPLE__)
    if (pluginDescriptor.id == pxr::TfToken("HdStormRendererPlugin")) {
        return "Metal";
    }
#endif

    return pluginDescriptor.displayName;
}

void Engine::SetCameraMatrices(pxr::GfMatrix4d view, pxr::GfMatrix4d proj)
{
    _camView = view;
    _camProj = proj;
}

void Engine::SetSelection(pxr::SdfPathVector paths)
{
    pxr::HdSelectionSharedPtr const selection =
        std::make_shared<pxr::HdSelection>();

    pxr::HdSelection::HighlightMode mode =
        pxr::HdSelection::HighlightModeSelect;

    for (auto&& path : paths) {
        pxr::SdfPath realPath = path.ReplacePrefix(
            pxr::SdfPath::AbsoluteRootPath(), _taskControllerId);
        selection->AddRprim(mode, realPath);
    }

    _selTracker->SetSelection(selection);
}

void Engine::SetRenderSize(int width, int height)
{
    _width = width;
    _height = height;

    _taskController->SetRenderViewport(pxr::GfVec4f(0, 0, width, height));
    _taskController->SetRenderBufferSize(pxr::GfVec2i(width, height));

    pxr::GfRange2f displayWindow(pxr::GfVec2f(0, 0),
                                 pxr::GfVec2f(width, height));
    pxr::GfRect2i renderBufferRect(pxr::GfVec2i(0, 0), width, height);
    pxr::GfRect2i dataWindow =
        renderBufferRect.GetIntersection(renderBufferRect);
    pxr::CameraUtilFraming framing(displayWindow, dataWindow);

    _taskController->SetFraming(framing);

    _drawTarget->Bind();
    _drawTarget->SetSize(pxr::GfVec2i(width, height));
    _drawTarget->Unbind();
}

void Engine::Present()
{
    pxr::VtValue aov;
    pxr::HgiTextureHandle aovTexture;

    if (_engine.GetTaskContextData(pxr::HdAovTokens->color, &aov)) {
        if (aov.IsHolding<pxr::HgiTextureHandle>()) {
            aovTexture = aov.Get<pxr::HgiTextureHandle>();
        }
    }

    uint32_t framebuffer = 0;
    _interop.TransferToApp(_hgi.get(), aovTexture,
                           /*srcDepth*/ pxr::HgiTextureHandle(),
                           pxr::HgiTokens->OpenGL, pxr::VtValue(framebuffer),
                           pxr::GfVec4i(0, 0, _width, _height));
}

void Engine::PrepareDefaultLighting()
{
    // set a spot light to the camera position
    pxr::GfVec3d camPos = _camView.GetInverse().ExtractTranslation();
    pxr::GlfSimpleLight l;
    l.SetAmbient(pxr::GfVec4f(0, 0, 0, 0));
    l.SetPosition(pxr::GfVec4f(camPos[0], camPos[1], camPos[2], 1));

    pxr::GlfSimpleMaterial material;
    material.SetAmbient(pxr::GfVec4f(2, 2, 2, 1.0));
    material.SetSpecular(pxr::GfVec4f(0.1, 0.1, 0.1, 1.0));
    material.SetShininess(32.0);

    pxr::GfVec4f sceneAmbient(0.01, 0.01, 0.01, 1.0);

    pxr::GlfSimpleLightingContextRefPtr lightingContextState =
        pxr::GlfSimpleLightingContext::New();

    lightingContextState->SetLights({l});
    lightingContextState->SetMaterial(material);
    lightingContextState->SetSceneAmbient(sceneAmbient);
    lightingContextState->SetUseLighting(true);
    _taskController->SetLightingState(lightingContextState);
}

void Engine::Prepare()
{
    PrepareDefaultLighting();

    _stageSceneIndex->ApplyPendingUpdates();
    _stageSceneIndex->SetTime(pxr::UsdTimeCode::Default());
    _taskController->SetFreeCameraMatrices(_camView, _camProj);
}

void Engine::Render()
{
    _drawTarget->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pxr::HdTaskSharedPtrVector tasks = _taskController->GetRenderingTasks();
    _engine.Execute(_renderIndex, &tasks);

    Present();

    _drawTarget->Unbind();
}

pxr::UsdPrim Engine::FindIntersection(pxr::GfVec2f screenPos)
{
    // create a narrowed frustum on the given position
    float normalizedXPos = screenPos[0] / _width;
    float normalizedYPos = screenPos[1] / _height;

    pxr::GfVec2d size(1.0 / _width, 1.0 / _height);

    pxr::GfCamera gfCam;
    gfCam.SetFromViewAndProjectionMatrix(_camView, _camProj);
    pxr::GfFrustum frustum = gfCam.GetFrustum();

    auto nFrustum = frustum.ComputeNarrowedFrustum(
        pxr::GfVec2d(2.0 * normalizedXPos - 1.0,
                     2.0 * (1.0 - normalizedYPos) - 1.0),
        size);

    // check the intersection from the narrowed frustum
    pxr::HdxPickHitVector allHits;
    pxr::HdxPickTaskContextParams pickParams;
    pickParams.resolveMode = pxr::HdxPickTokens->resolveNearestToCenter;
    pickParams.viewMatrix = nFrustum.ComputeViewMatrix();
    pickParams.projectionMatrix = nFrustum.ComputeProjectionMatrix();
    pickParams.collection = _collection;
    pickParams.outHits = &allHits;
    const pxr::VtValue vtPickParams(pickParams);

    _engine.SetTaskContextData(pxr::HdxPickTokens->pickParams, vtPickParams);

    // render with the picking task
    pxr::HdTaskSharedPtrVector tasks = _taskController->GetPickingTasks();
    _engine.Execute(_renderIndex, &tasks);

    // get the hitting point
    if (allHits.size() != 1) return pxr::UsdPrim();

    const pxr::SdfPath path = allHits[0].objectId.ReplacePrefix(
        _taskControllerId, pxr::SdfPath::AbsoluteRootPath());

    return _stage->GetPrimAtPath(path);
}

void* Engine::GetRenderBufferData()
{
    GLint id = _drawTarget->GetAttachment(pxr::HdAovTokens->color)
                   ->GetGlTextureName();
    return (void*)(uintptr_t)id;
}

pxr::GfFrustum Engine::GetFrustum()
{
    pxr::GfCamera gfCam;
    gfCam.SetFromViewAndProjectionMatrix(_camView, _camProj);
    return gfCam.GetFrustum();
}
