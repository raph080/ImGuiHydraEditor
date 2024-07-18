#include "gridsceneindex.h"

pxr::GridSceneIndex::GridSceneIndex()
{
    _gridPath = pxr::SdfPath("/Grid");
    _prim = _CreateGridPrim();
    Populate(true);
}

void pxr::GridSceneIndex::Populate(bool populate)
{
    if (populate && !_isPopulated) {
        _SendPrimsAdded({{_gridPath, HdPrimTypeTokens->basisCurves}});
    }
    else if (!populate && _isPopulated) {
        _SendPrimsRemoved({{_gridPath}});
    }
    _isPopulated = populate;
}

pxr::HdSceneIndexPrim pxr::GridSceneIndex::GetPrim(
    const pxr::SdfPath& primPath) const
{
    if (primPath == _gridPath) return _prim;
    else return {pxr::TfToken(), nullptr};
}

pxr::SdfPathVector pxr::GridSceneIndex::GetChildPrimPaths(
    const pxr::SdfPath& primPath) const
{
    if (!_isPopulated) return {};
    if (primPath == pxr::SdfPath::AbsoluteRootPath()) return {_gridPath};
    else return {};
}

pxr::HdSceneIndexPrim pxr::GridSceneIndex::_CreateGridPrim()
{
    pxr::VtVec3fArray colors = {};
    pxr::VtVec3fArray pts = {};
    pxr::VtIntArray vertexCounts = {};

    pxr::GfVec3f mainColor(.0f, .0f, .0f);
    pxr::GfVec3f subColor(.3f, .3f, .3f);

    int lines = 10;
    float padding = 1;

    pts.push_back(pxr::GfVec3f(0, 0, -padding * lines));
    pts.push_back(pxr::GfVec3f(0, 0, padding * lines));
    pts.push_back(pxr::GfVec3f(-padding * lines, 0, 0));
    pts.push_back(pxr::GfVec3f(padding * lines, 0, 0));

    for (int j = 0; j < 2; j++) {
        vertexCounts.push_back(2);
        colors.push_back(mainColor);
        colors.push_back(mainColor);
    }

    for (int i = 1; i <= lines; i++) {
        pts.push_back(pxr::GfVec3f(-padding * i, 0, -padding * lines));
        pts.push_back(pxr::GfVec3f(-padding * i, 0, padding * lines));
        pts.push_back(pxr::GfVec3f(padding * i, 0, -padding * lines));
        pts.push_back(pxr::GfVec3f(padding * i, 0, padding * lines));

        pts.push_back(pxr::GfVec3f(-padding * lines, 0, -padding * i));
        pts.push_back(pxr::GfVec3f(padding * lines, 0, -padding * i));
        pts.push_back(pxr::GfVec3f(-padding * lines, 0, padding * i));
        pts.push_back(pxr::GfVec3f(padding * lines, 0, padding * i));

        for (int j = 0; j < 4; j++) {
            vertexCounts.push_back(2);
            colors.push_back(subColor);
            colors.push_back(subColor);
        }
    }

    using _IntArrayDataSource =
        pxr::HdRetainedTypedSampledDataSource<pxr::VtIntArray>;
    using _TokenDataSource =
        pxr::HdRetainedTypedSampledDataSource<pxr::TfToken>;

    using _PointDataSource =
        pxr::HdRetainedTypedSampledDataSource<pxr::VtVec3fArray>;
    using _FloatDataSource =
        pxr::HdRetainedTypedSampledDataSource<pxr::VtFloatArray>;

    using _BoolDataSource = pxr::HdRetainedTypedSampledDataSource<bool>;

    using _MatrixDataSource =
        pxr::HdRetainedTypedSampledDataSource<pxr::GfMatrix4d>;

    pxr::HdSceneIndexPrim prim = pxr::HdSceneIndexPrim(
        {pxr::HdPrimTypeTokens->basisCurves,
         pxr::HdRetainedContainerDataSource::New(
             pxr::HdBasisCurvesSchemaTokens->basisCurves,
             pxr::HdBasisCurvesSchema::Builder()
                 .SetTopology(
                     pxr::HdBasisCurvesTopologySchema::Builder()
                         .SetCurveVertexCounts(
                             _IntArrayDataSource::New(vertexCounts))
                         .SetBasis(
                             _TokenDataSource::New(pxr::HdTokens->bezier))
                         .SetType(_TokenDataSource::New(pxr::HdTokens->linear))
                         .SetWrap(
                             _TokenDataSource::New(pxr::HdTokens->nonperiodic))
                         .Build())
                 .Build(),
             pxr::HdPrimvarsSchemaTokens->primvars,
             pxr::HdRetainedContainerDataSource::New(
                 pxr::HdPrimvarsSchemaTokens->points,
                 pxr::HdPrimvarSchema::Builder()
                     .SetPrimvarValue(_PointDataSource::New(pts))
                     .SetRole(pxr::HdPrimvarSchema::BuildRoleDataSource(
                         pxr::HdPrimvarSchemaTokens->point))
                     .SetInterpolation(
                         pxr::HdPrimvarSchema::BuildInterpolationDataSource(
                             pxr::HdPrimvarSchemaTokens->vertex))
                     .Build(),
                 pxr::HdPrimvarsSchemaTokens->widths,
                 pxr::HdPrimvarSchema::Builder()
                     .SetPrimvarValue(_FloatDataSource::New({1}))
                     .SetInterpolation(
                         pxr::HdPrimvarSchema::BuildInterpolationDataSource(
                             pxr::HdPrimvarSchemaTokens->constant))
                     .Build(),
                 pxr::HdTokens->displayColor,
                 pxr::HdPrimvarSchema::Builder()
                     .SetPrimvarValue(_PointDataSource::New(colors))
                     .SetInterpolation(
                         pxr::HdPrimvarSchema::BuildInterpolationDataSource(
                             pxr::HdPrimvarSchemaTokens->vertex))
                     .SetRole(pxr::HdPrimvarSchema::BuildRoleDataSource(
                         pxr::HdPrimvarSchemaTokens->color))
                     .Build()),
             pxr::HdPurposeSchemaTokens->purpose,
             pxr::HdPurposeSchema::Builder()
                 .SetPurpose(
                     _TokenDataSource::New(pxr::HdRenderTagTokens->geometry))
                 .Build(),
             pxr::HdVisibilitySchemaTokens->visibility,
             pxr::HdVisibilitySchema::Builder()
                 .SetVisibility(_BoolDataSource::New(true))
                 .Build(),
             pxr::HdXformSchemaTokens->xform,
             pxr::HdXformSchema::Builder()
                 .SetMatrix(_MatrixDataSource::New(pxr::GfMatrix4d(1)))
                 .SetResetXformStack(_BoolDataSource::New(false))
                 .Build())});
    return prim;
}
