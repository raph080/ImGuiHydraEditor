#include "colorfiltersceneindex.h"

#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>

#include <algorithm>

pxr::ColorFilterSceneIndex::ColorFilterSceneIndex(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

pxr::GfVec3f pxr::ColorFilterSceneIndex::GetDisplayColor(
    const pxr::SdfPath &primPath) const
{
    const pxr::VtValue *value =
        _colorDict.GetValueAtPath(primPath.GetAsString());
    if (value && !value->IsEmpty()) return value->Get<pxr::GfVec3f>();

    pxr::HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    pxr::HdPrimvarsSchema primvarsSchema =
        pxr::HdPrimvarsSchema::GetFromParent(prim.dataSource);

    if (!primvarsSchema.IsDefined()) return pxr::GfVec3f(-1.f);

    pxr::HdSampledDataSource::Time time(0);
    pxr::HdPrimvarSchema primvarSchema =
        primvarsSchema.GetPrimvar(pxr::HdTokens->displayColor);

    if (!primvarSchema.IsDefined()) return pxr::GfVec3f(-1.f);

    pxr::VtArray<pxr::GfVec3f> colors = primvarSchema.GetPrimvarValue()
                                            ->GetValue(time)
                                            .Get<pxr::VtArray<pxr::GfVec3f>>();

    if (colors.size() != 1) return pxr::GfVec3f(-1.f);

    return colors[0];
}

void pxr::ColorFilterSceneIndex::SetDisplayColor(const pxr::SdfPath &primPath,
                                                 pxr::GfVec3f color)
{
    _colorDict.SetValueAtPath(primPath.GetAsString(), pxr::VtValue(color));

    pxr::HdSceneIndexObserver::DirtiedPrimEntries entries;
    pxr::HdDataSourceLocator locator(pxr::HdPrimvarsSchemaTokens->primvars);
    entries.push_back({primPath, locator});

    _SendPrimsDirtied(entries);
}

pxr::HdSceneIndexPrim pxr::ColorFilterSceneIndex::GetPrim(
    const pxr::SdfPath &primPath) const
{
    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    pxr::GfVec3f color = GetDisplayColor(primPath);

    if (color == pxr::GfVec3f(-1.f)) return prim;

    prim.dataSource = pxr::HdOverlayContainerDataSource::New(
        pxr::HdRetainedContainerDataSource::New(
            pxr::HdPrimvarsSchemaTokens->primvars,
            pxr::HdRetainedContainerDataSource::New(
                pxr::HdTokens->displayColor,
                pxr::HdPrimvarSchema::Builder()
                    .SetPrimvarValue(pxr::HdRetainedTypedSampledDataSource<
                                     pxr::VtVec3fArray>::New({color}))
                    .SetInterpolation(
                        pxr::HdPrimvarSchema::BuildInterpolationDataSource(
                            pxr::HdPrimvarSchemaTokens->constant))
                    .SetRole(pxr::HdPrimvarSchema::BuildRoleDataSource(
                        pxr::HdPrimvarSchemaTokens->color))
                    .Build())),
        prim.dataSource);
    return prim;
}

pxr::SdfPathVector pxr::ColorFilterSceneIndex::GetChildPrimPaths(
    const pxr::SdfPath &primPath) const
{
    return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void pxr::ColorFilterSceneIndex::_PrimsAdded(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::AddedPrimEntries &entries)
{
    _SendPrimsAdded(entries);
}

void pxr::ColorFilterSceneIndex::_PrimsRemoved(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    _SendPrimsRemoved(entries);
}
void pxr::ColorFilterSceneIndex::_PrimsDirtied(
    const pxr::HdSceneIndexBase &sender,
    const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    _SendPrimsDirtied(entries);
}
