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

bool AreNearlyEquals(pxr::GfMatrix4f mat1, pxr::GfMatrix4f mat2,
                     float precision)
{
    pxr::GfMatrix4f subMat = mat1 - mat2;
    for (size_t i = 0; i < 16; i++)
        if (pxr::GfAbs(subMat.data()[i]) > precision) return false;
    return true;
}
