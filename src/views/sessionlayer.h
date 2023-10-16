#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <TextEditor.h>

#include "view.h"

using namespace std;

class SessionLayer : public View {
    public:
        inline static const string VIEW_TYPE = "Session Layer";

        SessionLayer(Model* model, const string label = VIEW_TYPE);

        const string GetViewType() override;
        void Draw() override;

    private:
        TextEditor editor;
        bool isEditing = false;
        string lastLoadedText;

        bool IsSessionLayerUpdated();

        void LoadTextFromModel();

        void SaveTextToModel();

        TextEditor::Palette GetPalette();

        TextEditor::LanguageDefinition GetLanguageDefinitionUSD();

        void FocusInEvent() override;
        void FocusOutEvent() override;
};
