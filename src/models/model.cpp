#include "model.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

Model::Model()
{
    _sceneIndices = pxr::HdMergingSceneIndex::New();
    _finalSceneIndex = pxr::HdMergingSceneIndex::New();
    _editableSceneIndex = _sceneIndices;
    SetEditableSceneIndex(_editableSceneIndex);

    pxr::UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    const pxr::UsdImagingSceneIndices sceneIndices =
        UsdImagingCreateSceneIndices(info);

    _stageSceneIndex = sceneIndices.stageSceneIndex;
    AddSceneIndex(sceneIndices.finalSceneIndex);
}

pxr::GfVec3d Model::GetUpAxis()
{
    pxr::TfToken upAxisToken = UsdGeomGetStageUpAxis(_stage);

    if (upAxisToken == pxr::UsdGeomTokens->x) return pxr::GfVec3d(1, 0, 0);
    if (upAxisToken == pxr::UsdGeomTokens->z) return pxr::GfVec3d(0, 0, 1);
    return pxr::GfVec3d(0, 1, 0);
}

pxr::UsdStageRefPtr Model::GetStage()
{
    return _stage;
}

void Model::SetStage(pxr::UsdStageRefPtr stage)
{
    _stage = stage;
    _stageSceneIndex->SetStage(_stage);
    _stageSceneIndex->SetTime(pxr::UsdTimeCode::Default());
}

void Model::AddSceneIndex(pxr::HdSceneIndexBaseRefPtr sceneIndex)
{
    _sceneIndices->AddInputScene(sceneIndex, pxr::SdfPath::AbsoluteRootPath());
}

pxr::HdSceneIndexBaseRefPtr Model::GetEditableSceneIndex()
{
    return _editableSceneIndex;
}

void Model::SetEditableSceneIndex(pxr::HdSceneIndexBaseRefPtr sceneIndex)
{
    _finalSceneIndex->RemoveInputScene(_editableSceneIndex);
    _editableSceneIndex = sceneIndex;
    _finalSceneIndex->AddInputScene(_editableSceneIndex,
                                    pxr::SdfPath::AbsoluteRootPath());
}

pxr::HdSceneIndexBaseRefPtr Model::GetFinalSceneIndex()
{
    return _finalSceneIndex;
}

void Model::ApplyModelUpdates()
{
    _stageSceneIndex->ApplyPendingUpdates();
}

pxr::HdSceneIndexPrim Model::GetPrim(pxr::SdfPath primPath)
{
    return _finalSceneIndex->GetPrim(primPath);
}

pxr::UsdPrim Model::GetUsdPrim(pxr::SdfPath path)
{
    return _stage->GetPrimAtPath(path);
}

pxr::UsdPrimRange Model::GetAllPrims()
{
    return _stage->Traverse();
}

pxr::SdfPathVector Model::GetCameras()
{
    pxr::UsdPrimSubtreeRange prims =
        _stage->GetPseudoRoot().GetAllDescendants();

    pxr::SdfPathVector camPaths;
    for (auto prim : prims) {
        if (prim.GetTypeName().GetString() == pxr::UsdGeomTokens->Camera) {
            camPaths.push_back(prim.GetPrimPath());
        }
    }
    return camPaths;
}

pxr::SdfPathVector Model::GetSelection()
{
    return _selection;
}

void Model::SetSelection(pxr::SdfPathVector primPaths)
{
    _selection = primPaths;
}
