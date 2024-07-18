/**
 * @file usd.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Header containing utility function to manipulate USD objects.
 *
 * @copyright Copyright (c) 2023
 *
 */
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
