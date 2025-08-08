#include "engine.h"

#include "backends/backend.h"

#include <iostream>

#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/imaging/hd/rendererPlugin.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hd/types.h>
#include <pxr/imaging/hdx/pickTask.h>
#include <pxr/imaging/hgi/tokens.h>
#include "pxr/imaging/hdSt/renderBuffer.h"

PXR_NAMESPACE_OPEN_SCOPE

Engine::Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin)
    : _sceneIndex(sceneIndex),
      _curRendererPlugin(plugin),
      _hgi(Hgi::CreatePlatformDefaultHgi()),
      _hgiDriver{HgiTokens->renderDriver, VtValue(_hgi.get())},
      _engine(),
      _renderDelegate(nullptr),
      _renderIndex(nullptr),
      _taskController(nullptr),
      _taskControllerId("/defaultTaskController"),
      _backendTexture(nullptr)
{
    _width = 512;
    _height = 512;

    _Initialize();
}

Engine::~Engine()
{
    _Clear();
}

void Engine::SetSceneIndex(HdSceneIndexBaseRefPtr newSceneIndex)
{
    if (_renderIndex && _sceneIndex) {
        _renderIndex->RemoveSceneIndex(_sceneIndex);
    }
    
    _sceneIndex = newSceneIndex;
    
    if (_renderIndex && _sceneIndex) {
        _renderIndex->InsertSceneIndex(_sceneIndex, _taskControllerId);
    }
}

HdSceneIndexBaseRefPtr Engine::GetSceneIndex() const
{
    return _sceneIndex;
}

TfTokenVector Engine::GetRendererPlugins()
{
    HfPluginDescVector pluginDescriptors;
    HdRendererPluginRegistry::GetInstance().GetPluginDescs(&pluginDescriptors);

    TfTokenVector plugins;
    for (size_t i = 0; i < pluginDescriptors.size(); ++i) {
        plugins.push_back(pluginDescriptors[i].id);
    }
    return plugins;
}

TfToken Engine::GetDefaultRendererPlugin()
{
    HdRendererPluginRegistry& registry =
        HdRendererPluginRegistry::GetInstance();
    return registry.GetDefaultPluginId(true);
}

TfToken Engine::GetCurrentRendererPlugin()
{
    return _curRendererPlugin;
}

string Engine::GetRendererPluginName(TfToken plugin)
{
    HfPluginDesc pluginDescriptor;
    bool foundPlugin = HdRendererPluginRegistry::GetInstance().GetPluginDesc(
        plugin, &pluginDescriptor);

    if (!foundPlugin) { return std::string(); }

    // TODO: fix that will be eventually delegate to Hgi
#if defined(__APPLE__)
    if (pluginDescriptor.id == TfToken("HdStormRendererPlugin")) {
        return "Metal";
    }
#endif

    return pluginDescriptor.displayName;
}

void Engine::SetRendererPlugin(TfToken newPluginId)
{
    auto sceneIndex = _sceneIndex;
    _Clear();
    _curRendererPlugin = newPluginId;
    _sceneIndex = sceneIndex;
    _Initialize();
}

void Engine::SetCameraMatrices(GfMatrix4d view, GfMatrix4d proj)
{
    _camView = view;
    _camProj = proj;
}

void Engine::SetSelection(SdfPathVector paths)
{
    HdSelectionSharedPtr const selection = std::make_shared<HdSelection>();

    HdSelection::HighlightMode mode = HdSelection::HighlightModeSelect;

    for (auto&& path : paths) {
        SdfPath realPath =
            path.ReplacePrefix(SdfPath::AbsoluteRootPath(), _taskControllerId);
        selection->AddRprim(mode, realPath);
    }

    _selTracker->SetSelection(selection);
}

void Engine::SetRenderSize(int width, int height)
{
    _width = width;
    _height = height;

    _taskController->SetRenderViewport(GfVec4f(0, 0, width, height));
    _taskController->SetRenderBufferSize(GfVec2i(width, height));

    GfRange2f displayWindow(GfVec2f(0, 0), GfVec2f(width, height));
    GfRect2i renderBufferRect(GfVec2i(0, 0), width, height);
    GfRect2i dataWindow = renderBufferRect.GetIntersection(renderBufferRect);
    CameraUtilFraming framing(displayWindow, dataWindow);

    _taskController->SetFraming(framing);
}

void Engine::Prepare()
{
    _PrepareDefaultLighting();
    _taskController->SetFreeCameraMatrices(_camView, _camProj);
}

void Engine::Render()
{
    // invalidate the previous cached backend texture
    if (_backendTexture) {
        DeleteTextureBackend(_backendTexture, _hgi.get());
        _backendTexture = nullptr;
    }

    _taskController->SetEnablePresentation(false);
    HdTaskSharedPtrVector tasks = _taskController->GetRenderingTasks();
    _engine.Execute(_renderIndex, &tasks);
}

SdfPath Engine::FindIntersection(GfVec2f screenPos)
{
    // create a narrowed frustum on the given position
    float normalizedXPos = screenPos[0] / _width;
    float normalizedYPos = screenPos[1] / _height;

    GfVec2d size(1.0 / _width, 1.0 / _height);

    GfCamera gfCam;
    gfCam.SetFromViewAndProjectionMatrix(_camView, _camProj);
    GfFrustum frustum = gfCam.GetFrustum();

    auto nFrustum = frustum.ComputeNarrowedFrustum(
        GfVec2d(2.0 * normalizedXPos - 1.0,
                2.0 * (1.0 - normalizedYPos) - 1.0),
        size);

    // check the intersection from the narrowed frustum
    HdxPickHitVector allHits;
    HdxPickTaskContextParams pickParams;
    pickParams.resolveMode = HdxPickTokens->resolveNearestToCenter;
    pickParams.viewMatrix = nFrustum.ComputeViewMatrix();
    pickParams.projectionMatrix = nFrustum.ComputeProjectionMatrix();
    pickParams.collection = _collection;
    pickParams.outHits = &allHits;
    const VtValue vtPickParams(pickParams);

    _engine.SetTaskContextData(HdxPickTokens->pickParams, vtPickParams);

    // render with the picking task
    HdTaskSharedPtrVector tasks = _taskController->GetPickingTasks();
    _engine.Execute(_renderIndex, &tasks);

    // get the hitting point
    if (allHits.size() != 1) return SdfPath();

    const SdfPath path = allHits[0].objectId.ReplacePrefix(
        _taskControllerId, SdfPath::AbsoluteRootPath());

    return path;
}

void* Engine::GetRenderBufferData()
{
    if (!_backendTexture) {
        auto buffer = _taskController->GetRenderOutput(HdAovTokens->color);
        _backendTexture = GetPointerToTextureBackend(buffer, _hgi.get());
    }

    return _backendTexture;
}

void Engine::_Clear()
{
    if (_taskController) {
        delete _taskController;
        _taskController = nullptr;
    }

    if (_renderIndex && _sceneIndex) {
        _renderIndex->RemoveSceneIndex(_sceneIndex);
        _sceneIndex = nullptr;
    }

    if (_renderIndex) {
        delete _renderIndex;
        _renderIndex = nullptr;
    }

    _renderDelegate = nullptr;

    if (_backendTexture) {
        DeleteTextureBackend(_backendTexture, _hgi.get());
        _backendTexture = nullptr;
    }
}

HdPluginRenderDelegateUniqueHandle Engine::_GetRenderDelegateFromPlugin(
    TfToken plugin)
{
    HdRendererPluginRegistry& registry =
        HdRendererPluginRegistry::GetInstance();

    TfToken resolvedId = registry.GetDefaultPluginId(true);

    return registry.CreateRenderDelegate(plugin);
}

void Engine::_Initialize()
{
    // init render delegate
    _renderDelegate = _GetRenderDelegateFromPlugin(_curRendererPlugin);

    // init render index
    _renderIndex = HdRenderIndex::New(_renderDelegate.Get(), {&_hgiDriver});

    _renderIndex->InsertSceneIndex(_sceneIndex, _taskControllerId);

    // init task controller

    _taskController = new HdxTaskController(_renderIndex, _taskControllerId);

    // init render paramss
    HdxRenderTaskParams params;
    params.viewport = GfVec4f(0, 0, _width, _height);
    params.enableLighting = true;
    _taskController->SetRenderParams(params);

    // init collection
    _collection = HdRprimCollection(HdTokens->geometry,
                                    HdReprSelector(HdReprTokens->smoothHull));
    _taskController->SetCollection(_collection);

    // init render tags
    _taskController->SetRenderTags(TfTokenVector());

    // init AOVs
    TfTokenVector _aovOutputs{HdAovTokens->color};
    _taskController->SetRenderOutputs(_aovOutputs);

    GfVec4f clearColor = GfVec4f(.2f, .2f, .2f, 1.0f);
    HdAovDescriptor colorAovDesc =
        _taskController->GetRenderOutputSettings(HdAovTokens->color);
    if (colorAovDesc.format != HdFormatInvalid) {
        colorAovDesc.clearValue = VtValue(clearColor);
        _taskController->SetRenderOutputSettings(HdAovTokens->color,
                                                 colorAovDesc);
    }

    // init selection
    GfVec4f selectionColor = GfVec4f(1.f, 1.f, 0.f, .5f);
    _selTracker = std::make_shared<HdxSelectionTracker>();

    _taskController->SetEnableSelection(true);
    _taskController->SetSelectionColor(selectionColor);

    VtValue selectionValue(_selTracker);
    _engine.SetTaskContextData(HdxTokens->selectionState, selectionValue);

    _taskController->SetOverrideWindowPolicy(CameraUtilFit);
}

void Engine::_PrepareDefaultLighting()
{
    // set a spot light to the camera position
    GfVec3d camPos = _camView.GetInverse().ExtractTranslation();
    GlfSimpleLight l;
    l.SetAmbient(GfVec4f(0, 0, 0, 0));
    l.SetPosition(GfVec4f(camPos[0], camPos[1], camPos[2], 1));

    GlfSimpleMaterial material;
    material.SetAmbient(GfVec4f(2, 2, 2, 1.0));
    material.SetSpecular(GfVec4f(0.1, 0.1, 0.1, 1.0));
    material.SetShininess(32.0);

    GfVec4f sceneAmbient(0.01, 0.01, 0.01, 1.0);

    GlfSimpleLightingContextRefPtr lightingContextState =
        GlfSimpleLightingContext::New();

    lightingContextState->SetLights({l});
    lightingContextState->SetMaterial(material);
    lightingContextState->SetSceneAmbient(sceneAmbient);
    lightingContextState->SetUseLighting(true);
    _taskController->SetLightingState(lightingContextState);
}

PXR_NAMESPACE_CLOSE_SCOPE