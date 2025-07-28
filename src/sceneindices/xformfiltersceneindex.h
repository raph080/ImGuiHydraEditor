/**
 * @file xformfiltersceneindex.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Hydra Filter Scene Index that overwrites xform of Hydra Prims.
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

class XformFilterSceneIndex;

TF_DECLARE_REF_PTRS(XformFilterSceneIndex);

/**
 * @class XformFilterSceneIndex
 * @brief Hydra Filter Scene Index that overwrites xform of Hydra Prims.
 *
 */
class XformFilterSceneIndex : public HdSingleInputFilteringSceneIndexBase {
    public:
        /**
         * @brief Create a ref pointer to a xform filter scene index
         *
         * @return XformFilterSceneIndexRefPtr the ref pointer to a xform
         * filter scene index
         */
        static XformFilterSceneIndexRefPtr New(
            const HdSceneIndexBaseRefPtr &inputSceneIndex)
        {
            return TfCreateRefPtr(
                new XformFilterSceneIndex(inputSceneIndex));
        }

        /**
         * @brief Construct a new Xform Filter Scene Index object
         *
         * @param inputSceneIndex the scene index to overwrite from
         */
        XformFilterSceneIndex(
            const HdSceneIndexBaseRefPtr &inputSceneIndex);

        /**
         * @brief Get the Xform of a hydra prim at the given path
         *
         * @param primPath the path of the prim to get the xform from
         * @return GfMatrix4d the xform of the prim
         */
        GfMatrix4d GetXform(const SdfPath &primPath) const;

        /**
         * @brief Set the Xform of a hydra prim at the given path
         *
         * @param primPath the path to the prim to set the xform
         * @param xform the new xform to set
         */
        void SetXform(const SdfPath &primPath, GfMatrix4d xform);

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
        VtDictionary _xformDict;
};

PXR_NAMESPACE_CLOSE_SCOPE