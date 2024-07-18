#include "model.h"

#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

#include <fstream>
#include <iostream>

Model::Model()
{
    SetEmptyStage();
}

Model::Model(const string usdFilePath)
{
    LoadUsdStage(usdFilePath);
}

void Model::SetEmptyStage()
{
    _stage = pxr::UsdStage::CreateInMemory();
    UsdGeomSetStageUpAxis(_stage, pxr::UsdGeomTokens->y);

    _rootLayer = _stage->GetRootLayer();
    _sessionLayer = _stage->GetSessionLayer();

    _stage->SetEditTarget(_sessionLayer);
}

void Model::LoadUsdStage(const string usdFilePath)
{
    if (!ifstream(usdFilePath)) {
        cout << "Error: the file does not exist. Empty stage loaded." << endl;
        SetEmptyStage();
        return;
    }

    _rootLayer = pxr::SdfLayer::FindOrOpen(usdFilePath);
    _sessionLayer = pxr::SdfLayer::CreateAnonymous();
    _stage = pxr::UsdStage::Open(_rootLayer, _sessionLayer);
    _stage->SetEditTarget(_sessionLayer);
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

pxr::SdfLayerRefPtr Model::GetSessionLayer()
{
    return _sessionLayer;
}