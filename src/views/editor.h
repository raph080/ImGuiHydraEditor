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

#include "sceneindices/colorfiltersceneindex.h"
#include "view.h"

PXR_NAMESPACE_OPEN_SCOPE

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
        SdfPath _prevSelection;
        ColorFilterSceneIndexRefPtr _colorFilterSceneIndex;

        /**
         * @brief Override of the View::Draw
         *
         */
        void _Draw() override;

        /**
         * @brief Get the SdfPath of the prim to display the attributes in the
         * editor.
         *
         * @return The current prim path to display the attributes from
         */
        SdfPath _GetPrimToDisplay();

        /**
         * @brief Append the display color attributes of the given prim path to
         * the editor view
         *
         * @param primPath the SdfPath to get the display color attributes from
         */
        void _AppendDisplayColorAttr(pxr::SdfPath primPath);

        /**
         * @brief Append the data source content to the editor view
         *
         * @param containerDataSource the container data source to display
         */
        void _AppendDataSourceAttrs(
            pxr::HdContainerDataSourceHandle containerDataSource);

        /**
         * @brief Append the display color attributes of the given prim
         * path to the editor view
         *
         * @param primPath the SdfPath to get the display color attributes
         * from
         */
        void _AppendAllPrimAttrs(pxr::SdfPath primPath);
};

PXR_NAMESPACE_CLOSE_SCOPE