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
#include <imgui_internal.h>
#include <pxr/usd/usd/prim.h>

#include "utils/usd.h"
#include "view.h"

using namespace std;

/**
 * @brief Outliner view that acts as an outliner. it allows to preview and
 * navigate into the UsdStage hierarchy.
 *
 */
class Outliner : public View {
    public:
        inline static const string VIEW_TYPE = "Outliner";

        /**
         * @brief Construct a new Outliner object
         *
         * @param model the Model of the new Outliner view
         * @param label the ImGui label of the new Outliner view
         */
        Outliner(Model* model, const string label = VIEW_TYPE);

        /**
         * @brief Override of the View::GetViewType
         *
         */
        const string GetViewType() override;

    private:
        /**
         * @brief Override of the View::Draw
         *
         */
        void Draw() override;

        /**
         * @brief Draw the hierarchy of all the descendant UsdPrims of the
         * given UsdPrim in the outliner
         *
         * @param prim the UsdPrim for which all the descendant hierarchy will
         * be drawn in the outliner
         * @return the ImRect rectangle of the tree node corresponding to the
         * given 'prim'
         */
        ImRect DrawPrimHierarchy(pxr::UsdPrim prim);

        /**
         * @brief Compute the display flags of the given UsdPrim
         *
         * @param prim the UsdPrim to compute the dislay flags from
         * @return an ImGuiTreeNodeFlags object.
         * Default is ImGuiTreeNodeFlags_None.
         * If 'prim' has no children, flag contains ImGuiTreeNodeFlags_Leaf
         * If 'prim' has children, flags contains
         * ImGuiTreeNodeFlags_OpenOnArrow
         * if 'prim' is part of selection, flags
         * contains ImGuiTreeNodeFlags_Selected
         *
         */
        ImGuiTreeNodeFlags ComputeDisplayFlags(pxr::UsdPrim prim);

        /**
         * @brief Draw the hierarchy tree node of the given UsdPrim. The color
         * and the behavior of the node will bet set accordingly.
         *
         * @param prim the UsdPrim that will be drawn next on the outliner
         * @return true if children 'prim' msut be drawn too
         * @return false otherwise
         */
        bool DrawHierarchyNode(pxr::UsdPrim prim);

        /**
         * @brief Check if the given UsdPrim is parent of a UsdPrim within the
         * current Model Selection.
         *
         * @param prim parent UsdPrim to check with
         * @return true if 'prim' is parent of a UsdPrim within the current
         * Model selection
         * @return false otherwise
         */
        bool IsParentOfModelSelection(pxr::UsdPrim prim);

        /**
         * @brief Check if the given UsdPrim is part of the current Model
         * selection
         *
         * @param prim UsdPrim to check with
         * @return true if 'prim' is part of the current Model selection
         * @return false otherwise
         */
        bool IsInModelSelection(pxr::UsdPrim prim);

        /**
         * @brief Draw the children hierarchy decoration of the outliner view
         * (aka the vertical and horizontal lines that connect parent and child
         * nodes).
         *
         * @param parentRect the ImRect rectangle of the parent node
         * @param childrenRects a vector of ImRect rectangles of the direct
         * children node of 'parentRect'
         */
        void DrawChildrendHierarchyDecoration(ImRect parentRect,
                                              vector<ImRect> childrenRects);
};