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

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

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
         * @brief Construct a new Model object and load the given Usd file as
         * root layer
         *
         * @param usdFilePath a string containing a Usd file path
         */
        Model(const string usdFilePath);

        /**
         * @brief Load a Usd Stage based on the given Usd file path
         *
         * @param usdFilePath a string containing a Usd file path
         */
        void LoadUsdStage(const string usdFilePath);

        /**
         * @brief Set the model to an empty stage
         *
         */
        void SetEmptyStage();

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
         * @brief Get a vector of all camera prims from the model
         *
         * @return vector<pxr::UsdPrim> a vector of camera prims
         */
        vector<pxr::UsdPrim> GetCameras();

        /**
         * @brief Get the current prim selection of the model
         *
         * @return vector<pxr::UsdPrim> a vector of the selected prims
         */
        vector<pxr::UsdPrim> GetSelection();

        /**
         * @brief Set the current prim selection of the model
         *
         * @param prims the vector containing the prim selection
         */
        void SetSelection(vector<pxr::UsdPrim> prims);

        /**
         * @brief Get a reference to the session layer of the model
         *
         * @return pxr::SdfLayerRefPtr a reference to the session layer
         */
        pxr::SdfLayerRefPtr GetSessionLayer();

    private:
        pxr::UsdStageRefPtr _stage;
        vector<pxr::UsdPrim> _selection;
        pxr::SdfLayerRefPtr _rootLayer, _sessionLayer;
};