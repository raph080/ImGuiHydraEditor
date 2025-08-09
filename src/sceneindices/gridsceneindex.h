/**
 * @file gridsceneindex.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Hydra Scene Index that creates a grid at the origin.
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <pxr/imaging/hd/sceneIndex.h>
#include "pxr/pxr.h"

PXR_NAMESPACE_OPEN_SCOPE

class GridSceneIndex;

TF_DECLARE_REF_PTRS(GridSceneIndex);

/**
 * @class GridSceneIndex
 * @brief Hydra Scene Index that creates a grid at the origin.
 *
 */
class GridSceneIndex : public HdSceneIndexBase {
    public:
        /**
         * @brief Create a ref pointer to a grid scene index
         *
         * @return GridSceneIndexRefPtr the ref pointer to a grid scene index
         */
        static GridSceneIndexRefPtr New()
        {
            return TfCreateRefPtr(new GridSceneIndex());
        }

        /**
         * @brief Construct a new grid scene index object
         *
         */
        GridSceneIndex();

        /**
         * @brief Populate the grid scene index
         *
         * @param populate true to populate the scene index, false otherwise
         */
        void Populate(bool populate);

        /**
         * @brief Get the prim at the given path
         *
         * @param primPath the path to a prim
         * @return HdSceneIndexPrim the hydra prim
         */
        virtual HdSceneIndexPrim GetPrim(const SdfPath& primPath) const;

        /**
         * @brief Get the child prim paths of a prim at the specified path
         * 
         * @param primPath the path of the prim the get the child paths from
         * @return SdfPathVector a list with all child prim paths
         */
        virtual SdfPathVector GetChildPrimPaths(const SdfPath& primPath) const;

    private:
        SdfPath _gridPath, _matPath, _prevSurfPath;
        HdSceneIndexPrim _gridPrim, _matPrim;
        bool _isPopulated;

        /**
         * @brief Create the grid hydra prim
         * 
         * @return HdSceneIndexPrim the hydra prim of the grid
         */
        HdSceneIndexPrim _CreateGridPrim();

        HdSceneIndexPrim _CreatePrevSurfPrim();
};

PXR_NAMESPACE_CLOSE_SCOPE