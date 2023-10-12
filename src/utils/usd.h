#pragma once

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/xformable.h>

#include <vector>

using namespace pxr;
using namespace std;

GfMatrix4d GetTransform(UsdGeomXformable geom)
{
    GfMatrix4d transform(1);
    bool resetsXformStack;
    transform = geom.ComputeLocalToWorldTransform(UsdTimeCode::Default());
    return transform;
};

void SetTransform(UsdGeomXformable geom, GfMatrix4d transform)
{
    GfMatrix4d parentTransform =
        geom.ComputeParentToWorldTransform(UsdTimeCode::Default());
    GfMatrix4d worldOffset = parentTransform.GetInverse() * transform;
    GfMatrix4d localOffset =
        parentTransform * worldOffset * parentTransform.GetInverse();
    geom.ClearXformOpOrder();
    geom.AddTransformOp().Set(localOffset);
};

bool IsParentOf(UsdPrim prim, UsdPrim childPrim)
{
    if (!prim.IsValid() || !childPrim.IsValid()) return false;

    UsdPrim curParent = childPrim.GetParent();
    while (curParent.IsValid()) {
        if (curParent == prim) return true;
        curParent = curParent.GetParent();
    }
    return false;
}

UsdPrim GetInstanceableParent(UsdPrim prim)
{
    if (!prim.IsValid()) return UsdPrim();

    UsdPrim curParent = prim.GetParent();
    UsdPrim lastInstanceablePrim = UsdPrim();
    while (curParent.IsValid()) {
        if (curParent.IsInstanceable()) lastInstanceablePrim = curParent;
        curParent = curParent.GetParent();
    }
    return lastInstanceablePrim;
}

bool AreNearlyEquals(GfMatrix4f mat1, GfMatrix4f mat2,
                     float precision = 0.001f)
{
    GfMatrix4f subMat = mat1 - mat2;
    for (size_t i = 0; i < 16; i++)
        if (GfAbs(subMat.data()[i]) > precision) return false;
    return true;
}