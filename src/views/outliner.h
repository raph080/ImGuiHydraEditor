#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <GraphEditor.h>
#include <ImGuizmo.h>
#include <boost/predef/os.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include "utils/usd.h"
#include "view.h"

using namespace std;

class Outliner : public View {
    public:
        inline static const string VIEW_TYPE = "Outliner";

        Outliner(Model* model, const string label = VIEW_TYPE)
            : View(model, label)
        {
        }
        const string GetViewType() override { return VIEW_TYPE; };

    private:
        void Draw() override
        {
            UsdStageRefPtr stage = GetModel()->GetStage();

            for (auto prim : stage->GetPseudoRoot().GetChildren()) {
                RenderTree(prim);
            }
        }

        // returns the node's rectangle
        ImRect RenderTree(UsdPrim prim)
        {
            UsdPrimSiblingRange children = prim.GetChildren();
            int childrenCount =
                std::distance(children.begin(), children.end());

            vector<UsdPrim> sel = GetModel()->GetSelection();
            bool isSelected = find(sel.begin(), sel.end(), prim) != sel.end();
            bool isParentOfSelection = false;
            for (auto&& p : sel) {
                if (IsParentOf(prim, p)) { isParentOfSelection = true; }
            }

            // construct the flags
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

            // set the flag if leaf or not
            if (childrenCount == 0) flags |= ImGuiTreeNodeFlags_Leaf;
            else flags = ImGuiTreeNodeFlags_OpenOnArrow;
            // if selected prim, set highlight flag

            if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

            bool recurse = false;
            const char* primName = prim.GetName().GetText();

            // print node in blue if parent of selection

            if (isParentOfSelection) {
                ImU32 color = ImGui::GetColorU32(ImGuiCol_HeaderActive, 1.f);
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                recurse = ImGui::TreeNodeEx(primName, flags);
                ImGui::PopStyleColor();
            }
            else {
                recurse = ImGui::TreeNodeEx(primName, flags);
            }

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                GetModel()->SetSelection({prim});
            }

            const ImRect nodeRect =
                ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

            if (recurse) {
                const ImColor TreeLineColor =
                    ImGui::GetColorU32(ImGuiCol_Text, 0.25f);
                const float SmallOffsetX =
                    -11.0f;  // for now, a hardcoded value; should
                             // take into account tree indent size
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
                verticalLineStart.x +=
                    SmallOffsetX;  // to nicely line up with the arrow symbol
                ImVec2 verticalLineEnd = verticalLineStart;

                for (auto child : children) {
                    const float HorizontalTreeLineSize =
                        8.0f;  // chosen arbitrarily
                    const ImRect childRect = RenderTree(child);
                    const float midpoint =
                        (childRect.Min.y + childRect.Max.y) / 2.0f;
                    drawList->AddLine(
                        ImVec2(verticalLineStart.x, midpoint),
                        ImVec2(verticalLineStart.x + HorizontalTreeLineSize,
                               midpoint),
                        TreeLineColor);
                    verticalLineEnd.y = midpoint;
                }

                drawList->AddLine(verticalLineStart, verticalLineEnd,
                                  TreeLineColor);
                ImGui::TreePop();
            }

            return nodeRect;
        }
};