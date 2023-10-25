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
         */
        MainWindow(Model* model);

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

    private:
        vector<View*> views;
        Model* model;

        /**
         * @brief Converts the given prim path by an indexed prim path if the
         * given prim path is already used in the Model. Indexing consist of
         * adding a number at the end of the path.
         *
         * @param primPath the given prim path to index if already exists in
         * Model
         * @return string the next index prim path
         */
        string GetNextAvailableIndexedPath(string primPath);

        /**
         * @brief Create a Camera in the Model
         *
         */
        void CreateCamera();

        /**
         * @brief Create a Capsule in the Model
         *
         */
        void CreateCapsule();

        /**
         * @brief Create a Cone in the Model
         *
         */
        void CreateCone();

        /**
         * @brief Create a Cube in the Model
         *
         */
        void CreateCube();

        /**
         * @brief Create a Cylinder in the Model
         *
         */
        void CreateCylinder();

        /**
         * @brief Create a Plane in the Model
         *
         */
        void CreatePlane();

        /**
         * @brief Create a Sphere in the Model
         *
         */
        void CreateSphere();
};