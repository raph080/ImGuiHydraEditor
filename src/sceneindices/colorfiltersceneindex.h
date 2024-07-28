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
        static pxr::ColorFilterSceneIndexRefPtr New(
            const pxr::HdSceneIndexBaseRefPtr &inputSceneIndex)
        {
            return TfCreateRefPtr(
                new pxr::ColorFilterSceneIndex(inputSceneIndex));
        }

        /**
         * @brief Construct a new Color Filter Scene Index object
         *
         * @param inputSceneIndex the scene index to overwrite from
         */
        ColorFilterSceneIndex(
            const pxr::HdSceneIndexBaseRefPtr &inputSceneIndex);

        /**
         * @brief Get the constant display color of a hydra prim at the given
         * path
         *
         * @param primPath the path of the prim to get the display color from
         * @return pxr::GfVec3f the display color of the prim
         */
        pxr::GfVec3f GetDisplayColor(const pxr::SdfPath &primPath) const;

        /**
         * @brief Set the constant display color of a hydra prim at the given
         * path
         *
         * @param primPath the path to the prim to set the display color
         * @param color the new constant display color to set
         */
        void SetDisplayColor(const pxr::SdfPath &primPath, pxr::GfVec3f color);

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::GetPrim
         */
        virtual pxr::HdSceneIndexPrim GetPrim(
            const pxr::SdfPath &primPath) const override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::GetChildPrimPaths
         */
        virtual pxr::SdfPathVector GetChildPrimPaths(
            const pxr::SdfPath &primPath) const override;

    protected:
        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsAdded
         */
        virtual void _PrimsAdded(
            const pxr::HdSceneIndexBase &sender,
            const pxr::HdSceneIndexObserver::AddedPrimEntries &entries)
            override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsRemoved
         */
        virtual void _PrimsRemoved(
            const pxr::HdSceneIndexBase &sender,
            const pxr::HdSceneIndexObserver::RemovedPrimEntries &entries)
            override;

        /**
         * @brief Override of
         * HdSingleInputFilteringSceneIndexBase::_PrimsDirtied
         */
        virtual void _PrimsDirtied(
            const pxr::HdSceneIndexBase &sender,
            const pxr::HdSceneIndexObserver::DirtiedPrimEntries &entries)
            override;

    private:
        pxr::VtDictionary _colorDict;
};

PXR_NAMESPACE_CLOSE_SCOPE