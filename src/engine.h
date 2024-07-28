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

#include <pxr/base/tf/token.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/imaging/hd/engine.h>
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include <pxr/imaging/hdx/taskController.h>
#include <pxr/imaging/hgi/hgi.h>
#include <pxr/imaging/hgiInterop/hgiInterop.h>
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
         * @brief Prepare the renderer
         */
        void Prepare();

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

    private:
        UsdStageRefPtr _stage;
        GlfDrawTargetRefPtr _drawTarget;
        GfMatrix4d _camView, _camProj;
        int _width, _height;

        HgiUniquePtr _hgi;
        HdDriver _hgiDriver;

        HdEngine _engine;
        HdPluginRenderDelegateUniqueHandle _renderDelegate;
        HdRenderIndex *_renderIndex;
        HdxTaskController *_taskController;
        HdRprimCollection _collection;
        HdSceneIndexBaseRefPtr _sceneIndex;
        SdfPath _taskControllerId;

        HgiInterop _interop;
        HdxSelectionTrackerSharedPtr _selTracker;

        TfToken _curRendererPlugin;

        /**
         * @brief Get the render delegate from the given renderer plugin
         *
         * @param plugin the renderer plugin
         *
         * @return the renderer delegate fro, the given renderer plugin
         */
        static HdPluginRenderDelegateUniqueHandle GetRenderDelegateFromPlugin(
            TfToken plugin);

        /**
         * @brief Initialize the renderer
         */
        void Initialize();

        /**
         * @brief Prepare the default lighting
         */
        void PrepareDefaultLighting();

        /**
         * @brief Present the last render to the OpenGL context
         */
        void Present();

        /**
         * @brief Get the current frustum
         *
         * @return the current frustum
         */
        GfFrustum GetFrustum();
};

PXR_NAMESPACE_CLOSE_SCOPE