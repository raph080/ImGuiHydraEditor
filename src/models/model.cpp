#include "model.h"

#include <pxr/imaging/hd/sceneIndexPrimView.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

PXR_NAMESPACE_OPEN_SCOPE

Model::Model()
{
    _sceneIndexBases = HdMergingSceneIndex::New();
    _finalSceneIndex = HdMergingSceneIndex::New();
    _editableSceneIndex = _sceneIndexBases;
    SetEditableSceneIndex(_editableSceneIndex);
}

UsdStageRefPtr Model::GetStage()
{
    return _stage;
}

void Model::SetStage(UsdStageRefPtr stage)
{
    _stage = stage;
}

void Model::AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex)
{
    _sceneIndexBases->AddInputScene(sceneIndex, SdfPath::AbsoluteRootPath());
}

HdSceneIndexBaseRefPtr Model::GetEditableSceneIndex()
{
    return _editableSceneIndex;
}

void Model::SetEditableSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
{
    _finalSceneIndex->RemoveInputScene(_editableSceneIndex);
    _editableSceneIndex = sceneIndex;
    _finalSceneIndex->AddInputScene(_editableSceneIndex,
                                    SdfPath::AbsoluteRootPath());
}

HdSceneIndexBaseRefPtr Model::GetFinalSceneIndex()
{
    return _finalSceneIndex;
}

HdSceneIndexPrim Model::GetPrim(SdfPath primPath)
{
    return _finalSceneIndex->GetPrim(primPath);
}

UsdPrim Model::GetUsdPrim(SdfPath path)
{
    return _stage->GetPrimAtPath(path);
}

UsdPrimRange Model::GetAllPrims()
{
    return _stage->Traverse();
}

SdfPathVector Model::GetCameras()
{
    SdfPath root = SdfPath::AbsoluteRootPath();
    HdSceneIndexPrimView primView(_finalSceneIndex, root);
    SdfPathVector camPaths;
    for (auto primPath : primView) {
        HdSceneIndexPrim prim = _finalSceneIndex->GetPrim(primPath);
        if (prim.primType == HdPrimTypeTokens->camera) {
            camPaths.push_back(primPath);
        }
    }
    return camPaths;
}

SdfPathVector Model::GetSelection()
{
    return _selection;
}

void Model::SetSelection(SdfPathVector primPaths)
{
    _selection = primPaths;
}

PXR_NAMESPACE_CLOSE_SCOPE
