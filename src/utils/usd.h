#pragma once

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/xformable.h>

#include <vector>

using namespace std;

/**
 * @brief Get the Transform Matrix of a UsdGeomXformable object
 *
 * @param geom the UsdGeomXformable object to get the matrix from
 * @return pxr::GfMatrix4d the Transform matrix of 'geom'
 */
pxr::GfMatrix4d GetTransformMatrix(pxr::UsdGeomXformable geom);

/**
 * @brief Set the Transform Matrix of USdGeomXformable object from a given
 * matrix
 *
 * @param geom the UsdGeomXformable object to set the transform matrix
 * @param transform the GfMatrix4d matrix that will be set to 'geom'
 */
void SetTransformMatrix(pxr::UsdGeomXformable geom, pxr::GfMatrix4d transform);

/**
 * @brief Check if a given prim is parent of another prim
 *
 * @param prim the UsdPrim to check if it is a parent from 'childPrim'
 * @param childPrim the UsdPrim that acts as the child prim
 * @return true if 'prim' is parent of 'childPrim'
 * @return false otherwise
 */
bool IsParentOf(pxr::UsdPrim prim, pxr::UsdPrim childPrim);

/**
 * @brief Get the first instanceable parent prim (from the root of the
 * UsdStage) of a given prim
 *
 * @param prim the UsdPrim to find the first instanceable parent from.
 * @return pxr::UsdPrim the first instanceable parent prim (from the root of
 * the stage) of 'prim'
 */
pxr::UsdPrim GetInstanceableParent(pxr::UsdPrim prim);

/**
 * @brief Check if two GfMatrix4f are nearly equal, based on a precision delta
 *
 * @param mat1 the first GfMatrix4f matrix to check with
 * @param mat2 the second GfMatrix4f matrix to check with
 * @param precision the precision delta for the comparison
 * @return true if the difference between the two matrices is less than
 * 'precision'.
 * @return false otherwise
 */
bool AreNearlyEquals(pxr::GfMatrix4f mat1, pxr::GfMatrix4f mat2,
                     float precision = 0.001f);

/**
 * @brief Get the size of a UsdPrimSiblingRange
 *
 * @param range the UsdPrimSiblingRange to get the size from
 * @return int the number of UsdPrims contained in the 'range'
 */
int GetSize(pxr::UsdPrimSiblingRange range);