#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "models/model.h"

using namespace std;

class View {
    public:
        inline static const string VIEW_TYPE = "View";

        View(Model* model, const string label);
        Model* GetModel();
        virtual const string GetViewType();
        const string GetViewLabel();
        void Update();
        bool IsDisplayed();

        virtual void ModelChangedEvent();

    protected:
        ImRect GetInnerRect();

    private:
        Model* model;
        string label;
        bool wasFocused = false;
        bool wasHovered = false;
        bool wasDisplayed = true;
        ImRect innerRect;
        ImVec2 _prevMousePos;

        virtual void Draw();
        virtual void FocusInEvent();
        virtual void FocusOutEvent();
        virtual void HoverInEvent();
        virtual void HoverOutEvent();
        virtual void KeyPressEvent(ImGuiKey key);
        virtual void MousePressEvent(ImGuiMouseButton_ button, ImVec2 pos);
        virtual void MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 pos);
        virtual void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos);

        virtual ImGuiWindowFlags GetGizmoWindowFlags();
};