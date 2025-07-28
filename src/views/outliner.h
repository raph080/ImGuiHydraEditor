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

#include "view.h"

PXR_NAMESPACE_OPEN_SCOPE

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
        void _Draw() override;

        /**
         * @brief Draw the hierarchy of all the descendant UsdPrims of the
         * given UsdPrim in the outliner
         *
         * @param primPath the SdfPath of the prim for which all the descendant
         * hierarchy will be drawn in the outliner
         * @return the ImRect rectangle of the tree node corresponding to the
         * given 'primPath'
         */
        ImRect _DrawPrimHierarchy(SdfPath primPath);

        /**
         * @brief Compute the display flags of the given UsdPrim
         *
         * @param primPath the SdfPath of the prim to compute the dislay flags
         * from
         * @return an ImGuiTreeNodeFlags object.
         * Default is ImGuiTreeNodeFlags_None.
         * If 'primPath' has no children, flag contains ImGuiTreeNodeFlags_Leaf
         * If 'primPath' has children, flags contains
         * ImGuiTreeNodeFlags_OpenOnArrow
         * if 'primPath' is part of selection, flags
         * contains ImGuiTreeNodeFlags_Selected
         *
         */
        ImGuiTreeNodeFlags _ComputeDisplayFlags(SdfPath primPath);

        /**
         * @brief Draw the hierarchy tree node of the given UsdPrim. The color
         * and the behavior of the node will bet set accordingly.
         *
         * @param primPath the SdfPath of the prim that will be drawn next on
         * the outliner
         * @return true if children 'primPath' must be drawn too
         * @return false otherwise
         */
        bool _DrawHierarchyNode(SdfPath primPath);

        /**
         * @brief Check if a given prim path is parent of another prim path
         *
         * @param primPath SdfPath to check if it is parent of
         * 'childPrimPath'
         * @param childPrimPath the SdfPath that acts as the child prim path
         * @return true if 'primPath' is parent of 'childPrimPath'
         * @return false otherwise
         */
        bool IsParentOf(SdfPath primPath, SdfPath childPrimPath);

        /**
         * @brief Check if the given UsdPrim is parent of a UsdPrim within the
         * current Model Selection.
         *
         * @param primPath SdfPath to check with
         * @return true if 'primPath' is parent of a prim within the current
         * Model selection
         * @return false otherwise
         */
        bool _IsParentOfModelSelection(SdfPath primPath);

        /**
         * @brief Check if the given UsdPrim is part of the current Model
         * selection
         *
         * @param prim SdfPath to check with
         * @return true if 'primPath' is part of the current Model selection
         * @return false otherwise
         */
        bool _IsInModelSelection(SdfPath primPath);

        /**
         * @brief Draw the children hierarchy decoration of the outliner view
         * (aka the vertical and horizontal lines that connect parent and child
         * nodes).
         *
         * @param parentRect the ImRect rectangle of the parent node
         * @param childrenRects a vector of ImRect rectangles of the direct
         * children node of 'parentRect'
         */
        void _DrawChildrendHierarchyDecoration(ImRect parentRect,
                                               vector<ImRect> childrenRects);
};

PXR_NAMESPACE_CLOSE_SCOPE