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

PXR_NAMESPACE_OPEN_SCOPE

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
         * @brief Get a reference to the Usd Stage from the model
         *
         * @return UsdStageRefPtr a reference to the Usd Stage
         */
        UsdStageRefPtr GetStage();

        /**
         * @brief Set a reference of a Usd Stage to the model
         *
         * @param stage UsdStageRefPtr a reference to the Usd Stage
         */
        void SetStage(UsdStageRefPtr stage);

        /**
         * @brief Add a Scene Index Base to the model
         *
         * @param sceneIndex HdSceneIndexBaseRefPtr Scene Index to add
         */
        void AddSceneIndexBase(HdSceneIndexBaseRefPtr sceneIndex);

        /**
         * @brief Get the Editable Scene Index from the model
         *
         * @return HdSceneIndexBaseRefPtr the editable Scene Index
         */
        HdSceneIndexBaseRefPtr GetEditableSceneIndex();

        /**
         * @brief Set the Editable Scene Index to the model
         *
         * @param sceneIndex the Editable Scene Index to set to the model
         */
        void SetEditableSceneIndex(HdSceneIndexBaseRefPtr sceneIndex);

        /**
         * @brief Get a reference to the Scene Index from the model
         *
         * @return HdSceneIndexBaseRefPtr a reference to the Scene Index
         */
        HdSceneIndexBaseRefPtr GetFinalSceneIndex();

        /**
         * @brief Get the Hydra Prim from the model at a specific path
         *
         * @param primPath SdfPath the path to the prim to get
         *
         * @return HdSceneIndexPrim the Hydra Prim at the given path
         */
        HdSceneIndexPrim GetPrim(SdfPath primPath);

        /**
         * @brief Get the Usd Prim from the model at a specific path
         *
         * @param primPath SdfPath the path to the prim to get
         *
         * @return UsdPrim the Usd Prim at the given path
         */
        UsdPrim GetUsdPrim(SdfPath primPath);

        /**
         * @brief Get the Usd Prim from the model
         *
         * @return UsdPrimRange a prim
         */
        UsdPrimRange GetAllPrims();

        /**
         * @brief Get a vector of all camera path from the model
         *
         * @return SdfPathVector a vector of camera paths
         */
        SdfPathVector GetCameras();

        /**
         * @brief Get the current prim selection of the model
         *
         * @return SdfPathVector a vector of the selected prim paths
         */
        SdfPathVector GetSelection();

        /**
         * @brief Set the current prim selection of the model
         *
         * @param primPaths the vector containing the prim paths selection
         */
        void SetSelection(SdfPathVector primPaths);

    private:
        UsdStageRefPtr _stage;
        SdfPathVector _selection;
        HdSceneIndexBaseRefPtr _editableSceneIndex;
        HdMergingSceneIndexRefPtr _sceneIndexBases, _finalSceneIndex;
};

PXR_NAMESPACE_CLOSE_SCOPE