#pragma once

#include <vector>

#include "models/model.h"
#include "views/view.h"

using namespace std;

class MainWindow {
    public:
        MainWindow(Model* model);

        void Update();

        void ResetDefaultViews();

        void AddView(const string viewType);

    private:
        vector<View*> views;
        Model* model;

        string GetNextAvailableIndexedPath(string primPath);

        void CreateCamera();

        void CreateCapsule();

        void CreateCone();

        void CreateCube();

        void CreateCylinder();

        void CreatePlane();

        void CreateSphere();
};