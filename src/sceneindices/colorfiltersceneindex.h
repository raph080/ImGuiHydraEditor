/**
 * @file colorfiltersceneindex.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Hydra Filter Scene Index that overwrites constant display color of
 * Hydra Prims.
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/vt/dictionary.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/pxr.h>

PXR_NAMESPACE_OPEN_SCOPE

class ColorFilterSceneIndex;

TF_DECLARE_REF_PTRS(ColorFilterSceneIndex);

/**
 * @class ColorFilterSceneIndex
 * @brief Hydra Filter Scene Index that overwrites constant display color of
 * Hydra Prims.
 */
class ColorFilterSceneIndex : public HdSingleInputFilteringSceneIndexBase {
    public:
        /**
         * @brief Create a ref pointer to a color filter scene index
         *
         * @return ColorFilterSceneIndexRefPtr the ref pointer to a color
         * filter scene index
         */
        static ColorFilterSceneIndexRefPtr New(
            const HdSceneIndexBaseRefPtr &inputSceneIndex)
        {
            return TfCreateRefPtr(
                new ColorFilterSceneIndex(inputSceneIndex));
        }

        /**
         * @brief Construct a new Color Filter Scene Index object
         *
         * @param inputSceneIndex the scene index to overwrite from
         */
        ColorFilterSceneIndex(
            const HdSceneIndexBaseRefPtr &inputSceneIndex);

        /**
         * @brief Get the constant display color of a hydra prim at the given
         * path
         *
         * @param primPath the path of the prim to get the display color from
         * @return GfVec3f the display color of the prim
         */
        GfVec3f GetDisplayColor(const SdfPath &primPath) const;

        /**
         * @brief Set the constant display color of a hydra prim at the given
         * path
         *
         * @param primPath the path to the prim to set the display color
         * @param color the new constant display color to set
         */
        void SetDisplayColor(const SdfPath &primPath, GfVec3f color);

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::GetPrim
         */
        virtual HdSceneIndexPrim GetPrim(
            const SdfPath &primPath) const override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::GetChildPrimPaths
         */
        virtual SdfPathVector GetChildPrimPaths(
            const SdfPath &primPath) const override;

    protected:
        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsAdded
         */
        virtual void _PrimsAdded(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::AddedPrimEntries &entries)
            override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsRemoved
         */
        virtual void _PrimsRemoved(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::RemovedPrimEntries &entries)
            override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsDirtied
         */
        virtual void _PrimsDirtied(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::DirtiedPrimEntries &entries)
            override;

    private:
        VtDictionary _colorDict;
};

PXR_NAMESPACE_CLOSE_SCOPE