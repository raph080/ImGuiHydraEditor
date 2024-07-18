#include "model.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

Model::Model()
{
    pxr::UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    const pxr::UsdImagingSceneIndices sceneIndices =
        UsdImagingCreateSceneIndices(info);

    _stageSceneIndex = sceneIndices.stageSceneIndex;
    _sceneIndex = sceneIndices.finalSceneIndex;
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

pxr::HdSceneIndexBaseRefPtr Model::GetSceneIndex()
{
    return _sceneIndex;
}

void Model::ApplyModelUpdates()
{
    _stageSceneIndex->ApplyPendingUpdates();
}

pxr::UsdPrim Model::GetPrim(pxr::SdfPath path)
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
