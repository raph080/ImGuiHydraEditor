/**
 * @file editor.h
 * @author Raphael Jouretz (rjouretz.com)
 * @brief Editor view that acts as an attribute editor. It allows to tune
 * attributes from selected UsdPrim.
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <pxr/usd/usd/prim.h>

#include "utils/usd.h"
#include "view.h"

using namespace std;

/**
 * @class Editor
 * @brief Editor view that acts as an attribute editor. It allows to tune
 * attributes from selected UsdPrim.
 *
 */
class Editor : public View {
    public:
        inline static const string VIEW_TYPE = "Editor";

        /**
         * @brief Construct a new Editor object
         *
         * @param model the Model of the new Editor view
         * @param label the ImGui label of the new Editor view
         */
        Editor(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

    private:
        pxr::UsdPrim _prevSelection;

        /**
         * @brief Override of the View::Draw
         *
         */
        void Draw() override;

        /**
         * @brief Get the UsdPrim to display the attributes in the editor.
         * The UsdPrim corresponds to the selected one or the previous selected
         * if the selection is empty
         *
         * @return The current prim to display the attributes from
         */
        pxr::UsdPrim GetPrimToDisplay();

        /**
         * @brief Append the transform attributes of the given prim to the
         * editor view
         *
         * @param prim the UsdPrim to get the transform attributes from
         */
        void AppendTransformAttrs(pxr::UsdPrim prim);

        /**
         * @brief Append the camera attributes of the given prim to the
         * editor view
         *
         * @param prim the UsdPrim to get the camera attributes from
         */
        void AppendCamAttrs(pxr::UsdPrim prim);

        /**
         * @brief Append the display color attributes of the given prim to the
         * editor view
         *
         * @param prim the UsdPrim to get the display color attributes from
         */
        void AppendDisplayColorAttr(pxr::UsdPrim prim);
};
