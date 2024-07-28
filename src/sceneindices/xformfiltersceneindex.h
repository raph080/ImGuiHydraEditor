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
        static pxr::XformFilterSceneIndexRefPtr New(
            const pxr::HdSceneIndexBaseRefPtr &inputSceneIndex)
        {
            return TfCreateRefPtr(
                new pxr::XformFilterSceneIndex(inputSceneIndex));
        }

        /**
         * @brief Construct a new Xform Filter Scene Index object
         *
         * @param inputSceneIndex the scene index to overwrite from
         */
        XformFilterSceneIndex(
            const pxr::HdSceneIndexBaseRefPtr &inputSceneIndex);

        /**
         * @brief Get the Xform of a hydra prim at the given path
         *
         * @param primPath the path of the prim to get the xform from
         * @return pxr::GfMatrix4d the xform of the prim
         */
        pxr::GfMatrix4d GetXform(const pxr::SdfPath &primPath) const;

        /**
         * @brief Set the Xform of a hydra prim at the given path
         *
         * @param primPath the path to the prim to set the xform
         * @param xform the new xform to set
         */
        void SetXform(const pxr::SdfPath &primPath, pxr::GfMatrix4d xform);

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
        pxr::VtDictionary _xformDict;
};

PXR_NAMESPACE_CLOSE_SCOPE