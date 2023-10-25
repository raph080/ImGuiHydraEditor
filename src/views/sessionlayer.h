/**
 * @file sessionlayer.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief SessionLayer view acts as a text editor for the session layer of the
 * current UsdStage. It allows to preview and edit the session layer.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <TextEditor.h>

#include "view.h"

using namespace std;

/**
 * @brief SessionLayer view acts as a text editor for the session layer of the
 * current UsdStage. It allows to preview and edit the USD session layer.
 *
 */
class SessionLayer : public View {
    public:
        inline static const string VIEW_TYPE = "Session Layer";

        /**
         * @brief Construct a new SessionLayer object
         *
         * @param model the Model of the new SessionLayer view
         * @param label the ImGui label of the new SessionLayer view
         */
        SessionLayer(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

    private:
        TextEditor editor;
        bool isEditing = false;
        string lastLoadedText;

        /**
         * @brief Override of the View::Draw
         *
         */
        void Draw() override;

        /**
         * @brief Check if USD session layer was updated since the last load
         * (different)
         *
         * @return true if USD session layer is different from the last loaded
         * data
         * @return false otherwise
         */
        bool IsSessionLayerUpdated();

        /**
         * @brief Load text from the USD session layer of the Model
         *
         */
        void LoadSessionTextFromModel();

        /**
         * @brief Save the text from the session layer view to the USD session
         * layer of the Model
         *
         */
        void SaveSessionTextToModel();

        /**
         * @brief Get a Palette object for the TextEditor (ImGui plugin)
         *
         * @return TextEditor::Palette
         */
        TextEditor::Palette GetPalette();

        /**
         * @brief Get the Usd Language Definition object for the TextEditor
         * (ImGui plugin)
         *
         * @return TextEditor::LanguageDefinition
         */
        TextEditor::LanguageDefinition GetUsdLanguageDefinition();

        /**
         * @brief Override of the View::FocusInEvent
         *
         */
        void FocusInEvent() override;

        /**
         * @brief Override of the View::FocusOutEvent
         *
         */
        void FocusOutEvent() override;
};
