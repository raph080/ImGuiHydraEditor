#include "colorfiltersceneindex.h"

#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>

#include <algorithm>

PXR_NAMESPACE_OPEN_SCOPE

ColorFilterSceneIndex::ColorFilterSceneIndex(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
    : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

GfVec3f ColorFilterSceneIndex::GetDisplayColor(const SdfPath &primPath) const
{
    const VtValue *value = _colorDict.GetValueAtPath(primPath.GetAsString());
    if (value && !value->IsEmpty()) return value->Get<GfVec3f>();

    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    HdPrimvarsSchema primvarsSchema =
        HdPrimvarsSchema::GetFromParent(prim.dataSource);

    if (!primvarsSchema.IsDefined()) return GfVec3f(-1.f);

    HdSampledDataSource::Time time(0);
    HdPrimvarSchema primvarSchema =
        primvarsSchema.GetPrimvar(HdTokens->displayColor);

    if (!primvarSchema.IsDefined()) return GfVec3f(-1.f);

    VtArray<GfVec3f> colors = primvarSchema.GetPrimvarValue()
                                  ->GetValue(time)
                                  .Get<VtArray<GfVec3f>>();

    if (colors.size() != 1) return GfVec3f(-1.f);

    return colors[0];
}

void ColorFilterSceneIndex::SetDisplayColor(const SdfPath &primPath,
                                            GfVec3f color)
{
    _colorDict.SetValueAtPath(primPath.GetAsString(), VtValue(color));

    HdSceneIndexObserver::DirtiedPrimEntries entries;
    HdDataSourceLocator locator(HdPrimvarsSchemaTokens->primvars);
    entries.push_back({primPath, locator});

    _SendPrimsDirtied(entries);
}

HdSceneIndexPrim ColorFilterSceneIndex::GetPrim(const SdfPath &primPath) const
{
    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    GfVec3f color = GetDisplayColor(primPath);

    if (color == GfVec3f(-1.f)) return prim;

    prim.dataSource = HdOverlayContainerDataSource::New(
        HdRetainedContainerDataSource::New(
            HdPrimvarsSchemaTokens->primvars,
            HdRetainedContainerDataSource::New(
                HdTokens->displayColor,
                HdPrimvarSchema::Builder()
                    .SetPrimvarValue(
                        HdRetainedTypedSampledDataSource<VtVec3fArray>::New(
                            {color}))
                    .SetInterpolation(
                        HdPrimvarSchema::BuildInterpolationDataSource(
                            HdPrimvarSchemaTokens->constant))
                    .SetRole(HdPrimvarSchema::BuildRoleDataSource(
                        HdPrimvarSchemaTokens->color))
                    .Build())),
        prim.dataSource);
    return prim;
}

SdfPathVector ColorFilterSceneIndex::GetChildPrimPaths(
    const SdfPath &primPath) const
{
    return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void ColorFilterSceneIndex::_PrimsAdded(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::AddedPrimEntries &entries)
{
    _SendPrimsAdded(entries);
}

void ColorFilterSceneIndex::_PrimsRemoved(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    _SendPrimsRemoved(entries);
}
void ColorFilterSceneIndex::_PrimsDirtied(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    _SendPrimsDirtied(entries);
}

PXR_NAMESPACE_CLOSE_SCOPE
