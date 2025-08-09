/**
 * @file outliner.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Outliner view that acts as an outliner. it allows to preview and
 * navigate into the UsdStage hierarchy.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <GraphEditor.h>
#include <imgui_internal.h>
#include <pxr/usd/usd/prim.h>

#include "view.h"

PXR_NAMESPACE_OPEN_SCOPE

using namespace std;

struct GraphEditorDelegate;

/**
 * @brief Outliner view that acts as an outliner. it allows to preview and
 * navigate into the UsdStage hierarchy.
 *
 */
class SceneIndexView : public View {
    public:
        inline static const string VIEW_TYPE = "Scene Index View";

        /**
         * @brief Construct a new SceneIndexView object
         *
         * @param model the Model of the new SceneIndexView view
         * @param label the ImGui label of the new SceneIndexView view
         */
        SceneIndexView(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

    private:
        ImGuiWindowFlags _gizmoWindowFlags;
        GraphEditor::FitOnScreen _fit;
        GraphEditor::Options _options;
        GraphEditor::ViewState _viewState;
        GraphEditorDelegate* delegate;

        /**
         * @brief Override of the View::Draw
         *
         */
        void _Draw() override;

};

PXR_NAMESPACE_CLOSE_SCOPE