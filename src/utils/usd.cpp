#include "usd.h"

pxr::GfMatrix4d GetTransformMatrix(pxr::UsdGeomXformable geom)
{
    pxr::GfMatrix4d transform(1);
    bool resetsXformStack;
    transform = geom.ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default());
    return transform;
};

void SetTransformMatrix(pxr::UsdGeomXformable geom, pxr::GfMatrix4d transform)
{
    pxr::GfMatrix4d parentTransform =
        geom.ComputeParentToWorldTransform(pxr::UsdTimeCode::Default());
    pxr::GfMatrix4d worldOffset = parentTransform.GetInverse() * transform;
    pxr::GfMatrix4d localOffset =
        parentTransform * worldOffset * parentTransform.GetInverse();
    geom.ClearXformOpOrder();
    geom.AddTransformOp().Set(localOffset);
};

bool IsParentOf(pxr::UsdPrim prim, pxr::UsdPrim childPrim)
{
    if (!prim.IsValid() || !childPrim.IsValid()) return false;

    pxr::UsdPrim curParent = childPrim.GetParent();
    while (curParent.IsValid()) {
        if (curParent == prim) return true;
        curParent = curParent.GetParent();
    }
    return false;
}

pxr::UsdPrim GetInstanceableParent(pxr::UsdPrim prim)
{
    if (!prim.IsValid()) return pxr::UsdPrim();

    pxr::UsdPrim curParent = prim.GetParent();
    pxr::UsdPrim lastInstanceablePrim = pxr::UsdPrim();
    while (curParent.IsValid()) {
        if (curParent.IsInstanceable()) lastInstanceablePrim = curParent;
        curParent = curParent.GetParent();
    }
    return lastInstanceablePrim;
}

bool AreNearlyEquals(pxr::GfMatrix4f mat1, pxr::GfMatrix4f mat2,
                     float precision)
{
    pxr::GfMatrix4f subMat = mat1 - mat2;
    for (size_t i = 0; i < 16; i++)
        if (pxr::GfAbs(subMat.data()[i]) > precision) return false;
    return true;
}

int GetSize(pxr::UsdPrimSiblingRange range)
{
    return std::distance(range.begin(), range.end());
}
