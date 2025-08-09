/**
 * @file engine.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Engine is the renderer that renders a stage according to a given
 * renderer plugin.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include "backends/backend.h"

#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_OPEN_SCOPE

using HgiUniquePtr = std::unique_ptr<class Hgi>;
using namespace std;

/**
 * @brief Engine is the renderer that renders a stage according to a given
 * renderer plugin.
 *
 */
class Engine {
    public:
        /**
         * @brief Construct a new Engine object
         *
         * @param sceneIndex the Scene Index to render
         * @param plugin the renderer plugin that specify the render
         */
        Engine(HdSceneIndexBaseRefPtr sceneIndex, TfToken plugin);

        /**
         * @brief Destroy the Engine object
         *
         */
        ~Engine();

        /**
         *  @brief Set The Scene Index to the Hydra Engine
         * 
         * @param newSceneIndex the new Scene Index to set
         */
        void SetSceneIndex(HdSceneIndexBaseRefPtr newSceneIndex);

        /**
         * @brief Get the active Scene Index
         *
         * @return the active Scene Index running in Hydra
         */
        HdSceneIndexBaseRefPtr GetSceneIndex() const;

        /**
         * @brief Get the list of available renderer plugins
         *
         * @return the available renderer plugins
         */
        static TfTokenVector GetRendererPlugins();

        /**
         * @brief Get the default renderer plugin (usually Storm)
         *
         * @return the default renderer plugin
         */
        static TfToken GetDefaultRendererPlugin();

        /**
         * @brief Get the name of a renderer plugin
         *
         * @return the name of a renderer plugin
         */
        string GetRendererPluginName(TfToken plugin);

        /**
         * @brief Get the current renderer plugin
         *
         * @return the current renderer plugin
         */
        TfToken GetCurrentRendererPlugin();

        /**
         * @brief Set Render plugin to Hydra Engine
         * 
         * @param newPluginId the new plugin Id to use in Hdyra
         */
        void SetRendererPlugin(TfToken newPluginId);

        /**
         * @brief Set the matrices of the current camera
         *
         * @param view the view matrix
         * @param proj the projection matrix
         */
        void SetCameraMatrices(GfMatrix4d view, GfMatrix4d proj);

        /**
         * @brief Set the current selection
         *
         * @param paths a vector of SDF Paths
         */
        void SetSelection(SdfPathVector paths);

        /**
         * @brief Set the render size
         *
         * @param width the width of the render
         * @param height the height of the render
         */
        void SetRenderSize(int width, int height);

        /**
         * @brief Render the current state
         */
        void Render();

        /**
         * @brief Find the visible USD Prim at the given screen position
         *
         * @param screenPos the position of the screen
         *
         * @return the Sdf Path to the Prim visible at the given screen
         * position
         */
        SdfPath FindIntersection(GfVec2f screenPos);

        /**
         * @brief Get the data from the render buffer
         *
         * @return the data from the render buffer
         */
        void *GetRenderBufferData();

        /**
         * @brief Set ambient light state
         * 
         * @param state true if ambient light is enabled
         */
        void SetAmbientLightEnabled(bool state);

        /**
         * @brief Set dome light state
         * 
         * @param state true if dome light is enabled
         */
        void SetDomeLightEnabled(bool state);

        /**
         * @brief Set dome light texture
         * 
         * @param texturePath file path to the dome light texture
         */
        void SetDomeLightTexturePath(string texturePath);

    private:
        UsdStageRefPtr _stage;
        GfMatrix4d _camView, _camProj;
        int _width, _height;

        HgiUniquePtr _hgi;
        HdDriver _hgiDriver;

        HdEngine _engine;
        HdPluginRenderDelegateUniqueHandle _renderDelegate;
        HdRenderIndex* _renderIndex;
        HdxTaskController* _taskController;
        HdRprimCollection _collection;
        HdSceneIndexBaseRefPtr _sceneIndex;
        SdfPath _taskControllerId;
        PresentTarget target;

        bool _domeLightEnabled, _ambientLightEnabled;
        string _domeLightTexturePath;

        HdxSelectionTrackerSharedPtr _selTracker;

        TfToken _curRendererPlugin;

        /**
         * @brief Clear Hydra Engine allocation and resources
         */
        void _Clear();

        /**
         * @brief Get the render delegate from the given renderer plugin
         *
         * @param plugin the renderer plugin
         *
         * @return the renderer delegate fro, the given renderer plugin
         */
        static HdPluginRenderDelegateUniqueHandle _GetRenderDelegateFromPlugin(
            TfToken plugin);

        /**
         * @brief Initialize the renderer
         */
        void _Initialize();

        /**
         * @brief Update the default lighting (ambient and dome lights)
         */
        void _UpdateLighting();
};

PXR_NAMESPACE_CLOSE_SCOPE