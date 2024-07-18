#include "xformfiltersceneindex.h"

#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hd/xformSchema.h>

pxr::XformFilterSceneIndex::XformFilterSceneIndex(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

pxr::GfMatrix4d pxr::XformFilterSceneIndex::GetXform(
    const pxr::SdfPath &primPath) const
{
    const pxr::VtValue *value =
        _xformDict.GetValueAtPath(primPath.GetAsString());
    if (value && !value->IsEmpty()) return value->Get<pxr::GfMatrix4d>();

    pxr::HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    pxr::HdXformSchema xformSchema =
        pxr::HdXformSchema::GetFromParent(prim.dataSource);
    if (!xformSchema.IsDefined()) return pxr::GfMatrix4d(1);

    pxr::HdSampledDataSource::Time time(0);
    pxr::GfMatrix4d xform =
        xformSchema.GetMatrix()->GetValue(time).Get<pxr::GfMatrix4d>();

    return xform;
}

void pxr::XformFilterSceneIndex::SetXform(const pxr::SdfPath &primPath,
                                          pxr::GfMatrix4d xform)
{
    _xformDict.SetValueAtPath(primPath.GetAsString(), pxr::VtValue(xform));

    pxr::HdSceneIndexObserver::DirtiedPrimEntries entries;
    entries.push_back({primPath, pxr::HdXformSchema::GetDefaultLocator()});

    _SendPrimsDirtied(entries);
}

pxr::HdSceneIndexPrim pxr::XformFilterSceneIndex::GetPrim(
    const pxr::SdfPath &primPath) const
{
    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    pxr::GfMatrix4d matrix = GetXform(primPath);

    prim.dataSource = pxr::HdOverlayContainerDataSource::New(
        pxr::HdRetainedContainerDataSource::New(
            pxr::HdXformSchemaTokens->xform,
            pxr::HdXformSchema::Builder()
                .SetMatrix(pxr::HdRetainedTypedSampledDataSource<
                           pxr::GfMatrix4d>::New(matrix))
                .SetResetXformStack(
                    pxr::HdRetainedTypedSampledDataSource<bool>::New(false))
                .Build()),
        prim.dataSource);

    return prim;
}

pxr::SdfPathVector pxr::XformFilterSceneIndex::GetChildPrimPaths(
    const pxr::SdfPath &primPath) const
{
    return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void pxr::XformFilterSceneIndex::_PrimsAdded(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::AddedPrimEntries &entries)
{
    _SendPrimsAdded(entries);
}

void pxr::XformFilterSceneIndex::_PrimsRemoved(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    _SendPrimsRemoved(entries);
}
void pxr::XformFilterSceneIndex::_PrimsDirtied(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    _SendPrimsDirtied(entries);
}
