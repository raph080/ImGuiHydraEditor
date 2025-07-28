#include "model.h"

#include <pxr/imaging/hd/sceneIndexPrimView.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

PXR_NAMESPACE_OPEN_SCOPE

Model::Model():
    _editableSceneIndex(nullptr),
    _activeSceneIndex(nullptr)
{
    _sceneIndexBases = HdMergingSceneIndex::New();
    _finalSceneIndex = HdMergingSceneIndex::New();
    SetEditableSceneIndex(_sceneIndexBases);
    SetActiveSceneIndex(_finalSceneIndex);

    _sceneIndexBases->SetDisplayName("SceneIndexBases");
    _finalSceneIndex->SetDisplayName("FinalSceneIndex");
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
    if (_editableSceneIndex){
        _finalSceneIndex->RemoveInputScene(_editableSceneIndex);
    }
    _editableSceneIndex = sceneIndex;
    _finalSceneIndex->AddInputScene(_editableSceneIndex,
                                    SdfPath::AbsoluteRootPath());
}

HdSceneIndexBaseRefPtr Model::GetFinalSceneIndex()
{
    return _finalSceneIndex;
}

void Model::SetActiveSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
{
    _activeSceneIndex = sceneIndex;
}

HdSceneIndexBaseRefPtr Model::GetActiveSceneIndex()
{
    return _activeSceneIndex;
}

HdSceneIndexPrim Model::GetPrim(SdfPath primPath)
{
    return _finalSceneIndex->GetPrim(primPath);
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
    // return empty list if selection corresponds to no prim
    // in the active scene index. Might be corresponding to
    // a prim from another scene index. Let's keep the value
    // of _selection for when the active scene index is updated
    SdfPathVector activeSelection = {};
    for ( auto primPath: _selection)
    {
        if (GetActiveSceneIndex()->GetPrim(primPath).dataSource)
            activeSelection.push_back(primPath);

    }
    return activeSelection;
}

void Model::SetSelection(SdfPathVector primPaths)
{
    _selection = primPaths;
}

PXR_NAMESPACE_CLOSE_SCOPE
