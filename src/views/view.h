/**
 * @file view.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief View represents an abstract view for the Main Window. Each class
 * inheriting from View can take advantage of convenient methods such as
 * events, flag definition and globals.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "models/model.h"

using namespace std;

/**
 * @brief View represents an abstract view for the Main Window. Each class
 * inheriting from View can take advantage of convenient methods such as
 * events, flag definition and globals.
 *
 */
class View {
    public:
        inline static const string VIEW_TYPE = "View";

        /**
         * @brief Construct a new View object
         *
         * @param model the Model of the new View view
         * @param label the ImGui label of the new View view
         */
        View(Model* model, const string label);

        /**
         * @brief Get the Model
         *
         * @return A pointer to the current model
         */
        Model* GetModel();

        /**
         * @brief Get the View Type of the current View object
         *
         * @return the view type
         */
        virtual const string GetViewType();

        /**
         * @brief Get the View Label of the current View object
         *
         * @return the view label
         */
        const string GetViewLabel();

        /**
         * @brief Update the current ImGui view and set the states for the
         * events
         *
         */
        void Update();

        /**
         * @brief Check if the current View object is displayed
         *
         * @return true if the view is displayed
         * @return false  otherwise
         */
        bool IsDisplayed();

        /**
         * @brief Called when the model changes
         *
         */
        virtual void ModelChangedEvent();

    protected:
        /**
         * @brief Get the inner rectangle object of the current ImGui view
         *
         * @return the rectangle object
         */
        ImRect GetInnerRect();

    private:
        Model* model;
        string label;
        bool wasFocused = false;
        bool wasHovered = false;
        bool wasDisplayed = true;
        ImRect innerRect;
        ImVec2 _prevMousePos;

        /**
         * @brief Called during the update of the view. Allow for custom draw
         *
         */
        virtual void Draw();

        /**
         * @brief Called when the view switch from unfocus to focus
         *
         */
        virtual void FocusInEvent();

        /**
         * @brief Called when the view swich from focus to unfocus
         *
         */
        virtual void FocusOutEvent();

        /**
         * @brief Called when the mouse enter the view
         *
         */
        virtual void HoverInEvent();

        /**
         * @brief Called when the mouse exit the view
         *
         */
        virtual void HoverOutEvent();

        /**
         * @brief Called when a key is press
         *
         * @param key the key that is pressed
         */
        virtual void KeyPressEvent(ImGuiKey key);

        /**
         * @brief Called when the mouse is pressed
         *
         * @param button the button that is pressed (left, middle, right, ...)
         * @param pos the position where the button is pressed
         */
        virtual void MousePressEvent(ImGuiMouseButton_ button, ImVec2 pos);

        /**
         * @brief Called when the mouse is released
         *
         * @param button the mouse button that is released (left, middle,
         * right, ...)
         * @param pos the position where the button is released
         */
        virtual void MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 pos);

        /**
         * @brief Called when the mouse move
         *
         * @param prevPos the previus position of the mouse
         * @param curPos the current position of the mouse
         */
        virtual void MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos);

        /**
         * @brief Get the ImGUi Window Flags that will be set to the current
         * view
         *
         * @return The ImGui Window flags
         */
        virtual ImGuiWindowFlags GetGizmoWindowFlags();
};