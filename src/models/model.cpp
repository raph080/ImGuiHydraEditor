#include "model.h"

#include <pxr/imaging/hd/sceneIndexPrimView.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

Model::Model()
{
    _sceneIndexBases = pxr::HdMergingSceneIndex::New();
    _finalSceneIndex = pxr::HdMergingSceneIndex::New();
    _editableSceneIndex = _sceneIndexBases;
    SetEditableSceneIndex(_editableSceneIndex);
}

pxr::UsdStageRefPtr Model::GetStage()
{
    return _stage;
}

void Model::SetStage(pxr::UsdStageRefPtr stage)
{
    _stage = stage;
}

void Model::AddSceneIndexBase(pxr::HdSceneIndexBaseRefPtr sceneIndex)
{
    _sceneIndexBases->AddInputScene(sceneIndex,
                                    pxr::SdfPath::AbsoluteRootPath());
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
    pxr::SdfPath root = pxr::SdfPath::AbsoluteRootPath();
    pxr::HdSceneIndexPrimView primView(_finalSceneIndex, root);
    pxr::SdfPathVector camPaths;
    for (auto primPath : primView) {
        pxr::HdSceneIndexPrim prim = _finalSceneIndex->GetPrim(primPath);
        if (prim.primType == pxr::HdPrimTypeTokens->camera) {
            camPaths.push_back(primPath);
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
