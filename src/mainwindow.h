/**
 * @file mainwindow.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Main window of the program that implements useful actions from the
 * menu bar and that manages all the views.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <vector>

#include "models/model.h"
#include "views/view.h"

PXR_NAMESPACE_OPEN_SCOPE

using namespace std;

/**
 * @brief Main window of the program that implements useful actions from the
 * menu bar and that manages all the views.
 *
 */
class MainWindow {
    public:
        /**
         * @brief Construct a new Main Window object
         *
         * @param model the Model of the new Main Window
         * @param application arguments
         */
        MainWindow(Model* model, const std::vector<std::string>& args);

        /**
         * @brief Update the draw call of the main window
         *
         */
        void Update();

        /**
         * @brief Reset the default views of the main window (1 x Outliner, 1
         * x Editor, 1 x Session Layer, 1 x Viewport)
         *
         *
         */
        void ResetDefaultViews();

        /**
         * @brief Add a new view of the given type to the main window
         *
         * @param viewType the type of the new view
         */
        void AddView(const string viewType);

        /**
        * @brief Gets views of given type
        *
        * @param viewType the type of the retrieved view
        */
        vector<View*> GetViewsOfType(const string viewType);

    private:

        vector<View*> _views;
        Model* _model;
};

PXR_NAMESPACE_CLOSE_SCOPE