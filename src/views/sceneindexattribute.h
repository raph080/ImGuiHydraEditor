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

#include "view.h"

PXR_NAMESPACE_OPEN_SCOPE

using namespace std;

/**
 * @class SceneIndexAttribute
 * @brief SceneIndexAttribute view that acts as an attribute viewer.
 *
 */
class SceneIndexAttribute : public View {
    public:
        inline static const string VIEW_TYPE = "Scene Index Attribute";

        /**
         * @brief Construct a new SceneIndexAttribute object
         *
         * @param model the Model of the new SceneIndexAttribute view
         * @param label the ImGui label of the new SceneIndexAttribute view
         */
        SceneIndexAttribute(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

    private:
        SdfPath _prevSelection;

        ImU32 NEW_ATTR_COL = IM_COL32(255, 165, 0, 255);
        ImU32 MODIFIED_ATTR_COL = IM_COL32(255, 69, 0, 255);
        ImU32 INHERITED_ATTR_COL;

        /**
         * @brief Override of the View::Draw
         *
         */
        void _Draw() override;

        /**
         * @brief draw the text color legend
         */
        void _DrawLegend();

        /**
         * @brief Get the SdfPath of the prim to display the attributes in the
         * SceneIndexAttribute.
         *
         * @return The current prim path to display the attributes from
         */
        SdfPath _GetPrimToDisplay();

        /**
         * @brief Append the data source content to the SceneIndexAttribute view
         *
         * @param containerDataSource the container data source to display
         * @param prevContainerDataSource the container data source of the previous scene index
         */
        void _AppendDataSourceAttrs(
            HdContainerDataSourceHandle containerDataSource,
            HdContainerDataSourceHandle prevContainerDataSource);

        /**
         * @brief Get color for the data source to be displayed.
         * 
         * The color will depends on the data being new, inherited or modified.
         *
         * @param containerDataSource the container data source to display
         * @param prevContainerDataSource the container data source of the previous scene index
         */
        ImU32 _GetDataSourceColor(
            HdDataSourceBaseHandle containerDataSource,
            HdDataSourceBaseHandle prevContainerDataSource);

        /**
         * @brief Append the display color attributes of the given prim
         * path to the SceneIndexAttribute view
         *
         * @param primPath the SdfPath to get the display color attributes
         * from
         */
        void _AppendAllPrimAttrs(SdfPath primPath);
};

PXR_NAMESPACE_CLOSE_SCOPE