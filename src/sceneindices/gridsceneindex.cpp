#include "gridsceneindex.h"

#include <pxr/imaging/hd/basisCurvesSchema.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/purposeSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usdImaging/usdImaging/tokens.h>
#include <pxr/imaging/hd/visibilitySchema.h>
#include <pxr/imaging/hd/xformSchema.h>
#include "pxr/imaging/hd/materialNodeSchema.h"
#include "pxr/imaging/hd/materialSchema.h"
#include "pxr/imaging/hd/materialNodeParameterSchema.h"
#include "pxr/imaging/hd/materialBindingSchema.h"
#include "pxr/imaging/hd/materialBindingsSchema.h"
#include "pxr/imaging/hd/materialConnectionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

GridSceneIndex::GridSceneIndex()
{
    SetDisplayName("GridSceneIndex");

    _gridPath = SdfPath("/Grid");
    _matPath = _gridPath.AppendPath(SdfPath("Material"));
    _prevSurfPath = _matPath.AppendPath(SdfPath("PreviewSurface"));
    _gridPrim = _CreateGridPrim();
    _matPrim = _CreatePrevSurfPrim();
    Populate(true);
}

void GridSceneIndex::Populate(bool populate)
{
    if (populate && !_isPopulated) {
        _SendPrimsAdded({
            { _matPath, HdPrimTypeTokens->material},
            {_gridPath, HdPrimTypeTokens->basisCurves}

            
        });
    }
    else if (!populate && _isPopulated) {
        _SendPrimsRemoved({{_gridPath},{_matPath}});
    }
    _isPopulated = populate;
}

HdSceneIndexPrim GridSceneIndex::GetPrim(const SdfPath& primPath) const
{
    if (primPath == _gridPath) return _gridPrim;
    if (primPath == _matPath) return _matPrim;
    else return {TfToken(), nullptr};
}

SdfPathVector GridSceneIndex::GetChildPrimPaths(const SdfPath& primPath) const
{
    if (!_isPopulated) return {};
    if (primPath == SdfPath::AbsoluteRootPath()) return {_gridPath};
    if (primPath == _gridPath) return {_matPath};
    else return {};
}

HdSceneIndexPrim GridSceneIndex::_CreateGridPrim()
{
    VtVec3fArray colors = {};
    VtVec3fArray pts = {};
    VtIntArray vertexCounts = {};

    GfVec3f mainColor(.0f, .0f, .0f);
    GfVec3f subColor(.3f, .3f, .3f);

    int lines = 10;
    float padding = 1;

    pts.push_back(GfVec3f(0, 0, -padding * lines));
    pts.push_back(GfVec3f(0, 0, padding * lines));
    pts.push_back(GfVec3f(-padding * lines, 0, 0));
    pts.push_back(GfVec3f(padding * lines, 0, 0));

    for (int j = 0; j < 2; j++) {
        vertexCounts.push_back(2);
        colors.push_back(mainColor);
    }

    for (int i = 1; i <= lines; i++) {
        pts.push_back(GfVec3f(-padding * i, 0, -padding * lines));
        pts.push_back(GfVec3f(-padding * i, 0, padding * lines));
        pts.push_back(GfVec3f(padding * i, 0, -padding * lines));
        pts.push_back(GfVec3f(padding * i, 0, padding * lines));

        pts.push_back(GfVec3f(-padding * lines, 0, -padding * i));
        pts.push_back(GfVec3f(padding * lines, 0, -padding * i));
        pts.push_back(GfVec3f(-padding * lines, 0, padding * i));
        pts.push_back(GfVec3f(padding * lines, 0, padding * i));

        for (int j = 0; j < 4; j++) {
            vertexCounts.push_back(2);
            colors.push_back(subColor);
        }
    }

    using _IntArrayDataSource = HdRetainedTypedSampledDataSource<VtIntArray>;
    using _TokenDataSource = HdRetainedTypedSampledDataSource<TfToken>;
    using _PointDataSource = HdRetainedTypedSampledDataSource<VtVec3fArray>;
    using _FloatDataSource = HdRetainedTypedSampledDataSource<VtFloatArray>;
    using _BoolDataSource = HdRetainedTypedSampledDataSource<bool>;
    using _MatrixDataSource = HdRetainedTypedSampledDataSource<GfMatrix4d>;
    using _PathDataSource = HdRetainedTypedSampledDataSource<SdfPath>;

    TfTokenVector purposes;
    std::vector<HdDataSourceBaseHandle> bindingsDs;

    purposes.push_back(HdMaterialBindingsSchemaTokens->allPurpose);
    bindingsDs.push_back(
            HdMaterialBindingSchema::Builder()
            .SetPath(_PathDataSource::New(_matPath))
            .Build());

    HdSceneIndexPrim prim = HdSceneIndexPrim({
        HdPrimTypeTokens->basisCurves,
        HdRetainedContainerDataSource::New(
            HdBasisCurvesSchemaTokens->basisCurves,
            HdBasisCurvesSchema::Builder()
                .SetTopology(
                    HdBasisCurvesTopologySchema::Builder()
                        .SetCurveVertexCounts(
                            _IntArrayDataSource::New(vertexCounts))
                        .SetBasis(_TokenDataSource::New(HdTokens->bezier))
                        .SetType(_TokenDataSource::New(HdTokens->linear))
                        .SetWrap(_TokenDataSource::New(HdTokens->nonperiodic))
                        .Build())
                .Build(),
            HdPrimvarsSchemaTokens->primvars,
            HdRetainedContainerDataSource::New(
                HdPrimvarsSchemaTokens->points,
                HdPrimvarSchema::Builder()
                    .SetPrimvarValue(_PointDataSource::New(pts))
                    .SetRole(HdPrimvarSchema::BuildRoleDataSource(
                        HdPrimvarSchemaTokens->point))
                    .SetInterpolation(
                        HdPrimvarSchema::BuildInterpolationDataSource(
                            HdPrimvarSchemaTokens->vertex))
                    .Build(),
                HdPrimvarsSchemaTokens->widths,
                HdPrimvarSchema::Builder()
                    .SetPrimvarValue(_FloatDataSource::New({1}))
                    .SetInterpolation(
                        HdPrimvarSchema::BuildInterpolationDataSource(
                            HdPrimvarSchemaTokens->constant))
                    .Build()
            ),
            HdPurposeSchemaTokens->purpose,
            HdPurposeSchema::Builder()
                .SetPurpose(
                    _TokenDataSource::New(HdRenderTagTokens->geometry))
                .Build(),
            HdVisibilitySchemaTokens->visibility,
            HdVisibilitySchema::Builder()
                .SetVisibility(_BoolDataSource::New(true))
                .Build(),
            HdXformSchemaTokens->xform,
            HdXformSchema::Builder()
                .SetMatrix(_MatrixDataSource::New(GfMatrix4d(1)))
                .SetResetXformStack(_BoolDataSource::New(false))
                .Build(),
            HdMaterialBindingsSchemaTokens->materialBindings,
            HdMaterialBindingsSchema::BuildRetained(
                1,
                purposes.data(),
                bindingsDs.data()
            )
        )});
    return prim;
}

HdSceneIndexPrim GridSceneIndex::_CreatePrevSurfPrim()
{
    using _TokenDataSource  = HdRetainedTypedSampledDataSource<TfToken>;
    using _FloatDataSource  = HdRetainedTypedSampledDataSource<float>;
    using _Vec3fDataSource  = HdRetainedTypedSampledDataSource<GfVec3f>;
    using _PathDataSource   = HdRetainedTypedSampledDataSource<SdfPath>;

    auto one = HdMaterialNodeParameterSchema::Builder()
        .SetValue(_FloatDataSource::New(1.0f)).Build();
    auto zero =HdMaterialNodeParameterSchema::Builder()
        .SetValue(_FloatDataSource::New(.0f)).Build();
    auto black = HdMaterialNodeParameterSchema::Builder()
        .SetValue(_Vec3fDataSource::New(GfVec3f(.0f, .0f, .0f))).Build();

    auto surfaceNode = HdMaterialNodeSchema::Builder()
        .SetNodeIdentifier(_TokenDataSource::New(UsdImagingTokens->UsdPreviewSurface))
        .SetParameters(HdRetainedContainerDataSource::New(
            TfToken("emissiveColor"), black,
            TfToken("diffuseColor"),  black,
            TfToken("specular"),      zero,
            TfToken("metallic"),      zero,
            TfToken("roughness"),     one,
            TfToken("opacity"),       one))
        .SetInputConnections(HdRetainedContainerDataSource::New()).Build();

    auto terminals = HdRetainedContainerDataSource::New(
        HdMaterialTerminalTokens->surface,
        HdMaterialConnectionSchema::Builder()
            .SetUpstreamNodePath(
                _TokenDataSource::New(_prevSurfPath.GetToken()))
            .SetUpstreamNodeOutputName(
                _TokenDataSource::New(HdMaterialTerminalTokens->surface))
            .Build()
    );

    std::vector<TfToken> networkNames;
    std::vector<HdDataSourceBaseHandle> networks;

    networkNames.push_back(HdMaterialSchemaTokens->universalRenderContext);
    networks.push_back(
        HdMaterialNetworkSchema::Builder()
        .SetNodes( HdRetainedContainerDataSource::New(
                _prevSurfPath.GetToken(),
                surfaceNode))
        .SetTerminals(terminals)
        .Build()
    );

    auto material = HdRetainedContainerDataSource::New(
        HdMaterialSchemaTokens->material,
        HdMaterialSchema::BuildRetained(
            networkNames.size(),
            networkNames.data(),
            networks.data()
        )
    );

    HdSceneIndexPrim prim = HdSceneIndexPrim({
        HdPrimTypeTokens->material,
        material
    });

    return prim;
}

PXR_NAMESPACE_CLOSE_SCOPE
