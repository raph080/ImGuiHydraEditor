/**
 * @file model.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Model containing the current state of the program such as the USD
 * stage, the session layer, the current selection, and associated data.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <pxr/base/gf/vec3d.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

#include <vector>

using namespace std;

/**
 * @brief Model containing the current state of the program such as the USD
 * stage, the session layer, the current selection, and associated data.
 *
 */
class Model {
    public:
        /**
         * @brief Construct a new Model object
         *
         */
        Model();

        /**
         * @brief Get the Up Axis of the model
         *
         * @return the up vector
         */
        pxr::GfVec3d GetUpAxis();

        /**
         * @brief Get a reference to the Usd Stage from the model
         *
         * @return pxr::UsdStageRefPtr a reference to the Usd Stage
         */
        pxr::UsdStageRefPtr GetStage();

        /**
         * @brief Set a reference of a Usd Stage to the model
         *
         * @param stage pxr::UsdStageRefPtr a reference to the Usd Stage
         */
        void SetStage(pxr::UsdStageRefPtr stage);

        /**
         * @brief Add a Scene Index Base to the model
         *
         * @param sceneIndex pxr::HdSceneIndexBaseRefPtr Scene Index to add
         */
        void AddSceneIndexBase(pxr::HdSceneIndexBaseRefPtr sceneIndex);

        /**
         * @brief Get the Editable Scene Index from the model
         *
         * @return pxr::HdSceneIndexBaseRefPtr the editable Scene Index
         */
        pxr::HdSceneIndexBaseRefPtr GetEditableSceneIndex();

        /**
         * @brief Set the Editable Scene Index to the model
         *
         * @param sceneIndex the Editable Scene Index to set to the model
         */
        void SetEditableSceneIndex(pxr::HdSceneIndexBaseRefPtr sceneIndex);

        /**
         * @brief Get a reference to the Scene Index from the model
         *
         * @return pxr::HdSceneIndexBaseRefPtr a reference to the Scene Index
         */
        pxr::HdSceneIndexBaseRefPtr GetFinalSceneIndex();

        /**
         * @brief Get the Hydra Prim from the model at a specific path
         *
         * @param primPath pxr::SdfPath the path to the prim to get
         *
         * @return pxr::HdSceneIndexPrim the Hydra Prim at the given path
         */
        pxr::HdSceneIndexPrim GetPrim(pxr::SdfPath primPath);

        /**
         * @brief Get the Usd Prim from the model at a specific path
         *
         * @param primPath pxr::SdfPath the path to the prim to get
         *
         * @return pxr::UsdPrim the Usd Prim at the given path
         */
        pxr::UsdPrim GetUsdPrim(pxr::SdfPath primPath);

        /**
         * @brief Get the Usd Prim from the model
         *
         * @return pxr::UsdPrimRange a prim
         */
        pxr::UsdPrimRange GetAllPrims();

        /**
         * @brief Get a vector of all camera path from the model
         *
         * @return pxr::SdfPathVector a vector of camera paths
         */
        pxr::SdfPathVector GetCameras();

        /**
         * @brief Get the current prim selection of the model
         *
         * @return pxr::SdfPathVector a vector of the selected prim paths
         */
        pxr::SdfPathVector GetSelection();

        /**
         * @brief Set the current prim selection of the model
         *
         * @param primPaths the vector containing the prim paths selection
         */
        void SetSelection(pxr::SdfPathVector primPaths);

    private:
        pxr::UsdStageRefPtr _stage;
        pxr::SdfPathVector _selection;
        pxr::HdSceneIndexBaseRefPtr _editableSceneIndex;
        pxr::HdMergingSceneIndexRefPtr _sceneIndexBases, _finalSceneIndex;
};