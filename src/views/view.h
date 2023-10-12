#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "models/model.h"

using namespace std;

class View {
    public:
        inline static const string VIEW_TYPE = "View";

        View(Model* model, const string label) : model(model), label(label){};
        Model* GetModel() { return model; }
        const string GetViewLabel() { return label; }
        void Update()
        {
            ImGuiIO& io = ImGui::GetIO();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
            ImGui::Begin(label.c_str(), &wasDisplayed, GetGizmoWindowFlags());

            // update focus state
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows |
                                       ImGuiFocusedFlags_RootWindow)) {
                if (!wasFocused) {
                    FocusInEvent();
                    wasFocused = true;
                }
            }
            else if (wasFocused) {
                FocusOutEvent();
                wasFocused = false;
            }

            if (wasFocused) {
                int key = ImGuiKey_NamedKey_BEGIN;
                while (key < ImGuiKey_NamedKey_END) {
                    if (ImGui::IsKeyPressed((ImGuiKey)key))
                        KeyPressEvent((ImGuiKey)key);
                    key++;
                }
            }

            Draw();

            // update inner rect size
            innerRect = ImGui::GetCurrentWindow()->InnerRect;

            if (ImGui::IsMouseHoveringRect(innerRect.Min, innerRect.Max)) {
                if (!wasHovered) {
                    HoverInEvent();
                    wasHovered = true;
                }
            }
            else if (wasHovered) {
                HoverOutEvent();
                wasHovered = false;
            }

            if (wasHovered) {
                // update mouse pos
                ImVec2 appMousePos = ImGui::GetMousePos();
                ImVec2 viewMousePos = appMousePos - innerRect.Min;

                MouseMoveEvent(_prevMousePos, viewMousePos);
                _prevMousePos = viewMousePos;

                for (int i = 0; i < 5; i++) {
                    if (io.MouseClicked[i])
                        MousePressEvent((ImGuiMouseButton_)i, viewMousePos);
                    if (io.MouseReleased[i])
                        MouseReleaseEvent((ImGuiMouseButton_)i, viewMousePos);
                }
            }

            ImGui::End();
            ImGui::PopStyleVar(2);
        };

        bool IsDisplayed() { return wasDisplayed; }

        virtual const string GetViewType() { return VIEW_TYPE; };
        virtual void ModelChangedEvent(){};

    protected:
        ImRect GetInnerRect() { return innerRect; }

    private:
        Model* model;
        string label;
        bool wasFocused = false;
        bool wasHovered = false;
        bool wasDisplayed = true;
        ImRect innerRect;
        ImVec2 _prevMousePos;

        virtual void Draw(){};
        virtual void FocusInEvent(){};
        virtual void FocusOutEvent(){};
        virtual void HoverInEvent(){};
        virtual void HoverOutEvent(){};
        virtual void KeyPressEvent(ImGuiKey key){};
        virtual void MousePressEvent(ImGuiMouseButton_ button, ImVec2 pos){};
        virtual void MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 pos){};
        virtual void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos){};

        virtual ImGuiWindowFlags GetGizmoWindowFlags()
        {
            return ImGuiWindowFlags_None;
        };
};