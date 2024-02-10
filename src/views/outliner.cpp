#include "outliner.h"

#include <pxr/usd/usd/stage.h>

Outliner::Outliner(Model* model, const string label) : View(model, label) {}

const string Outliner::GetViewType()
{
    return VIEW_TYPE;
};

void Outliner::_Draw()
{
    pxr::UsdStageRefPtr stage = GetModel()->GetStage();

    for (auto prim : stage->GetPseudoRoot().GetChildren()) {
        _DrawPrimHierarchy(prim);
    }
}

// returns the node's rectangle
ImRect Outliner::_DrawPrimHierarchy(pxr::UsdPrim prim)
{
    bool recurse = _DrawHierarchyNode(prim);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        GetModel()->SetSelection({prim});
    }

    const ImRect curItemRect =
        ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    if (recurse) {
        // draw all children and store their rect position
        vector<ImRect> rects;
        for (auto child : prim.GetChildren()) {
            ImRect childRect = _DrawPrimHierarchy(child);
            rects.push_back(childRect);
        }

        if (rects.size() > 0) {
            // draw hierarchy decoration for all children
            _DrawChildrendHierarchyDecoration(curItemRect, rects);
        }

        ImGui::TreePop();
    }

    return curItemRect;
}

ImGuiTreeNodeFlags Outliner::_ComputeDisplayFlags(pxr::UsdPrim prim)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

    // set the flag if leaf or not
    pxr::UsdPrimSiblingRange children = prim.GetChildren();
    if (GetSize(children) == 0) {
        flags |= ImGuiTreeNodeFlags_Leaf;
        flags |= ImGuiTreeNodeFlags_Bullet;
    }
    else flags = ImGuiTreeNodeFlags_OpenOnArrow;

    // if selected prim, set highlight flag
    bool isSelected = _IsInModelSelection(prim);
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    return flags;
}

bool Outliner::_DrawHierarchyNode(pxr::UsdPrim prim)
{
    bool recurse = false;
    const char* primName = prim.GetName().GetText();
    ImGuiTreeNodeFlags flags = _ComputeDisplayFlags(prim);

    // print node in blue if parent of selection
    if (_IsParentOfModelSelection(prim)) {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_HeaderActive, 1.f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        recurse = ImGui::TreeNodeEx(primName, flags);
        ImGui::PopStyleColor();
    }
    else {
        recurse = ImGui::TreeNodeEx(primName, flags);
    }
    return recurse;
}

bool Outliner::_IsParentOfModelSelection(pxr::UsdPrim prim)
{
    // check if prim is parent of selection
    for (auto&& p : GetModel()->GetSelection())
        if (IsParentOf(prim, p)) return true;

    return false;
}

bool Outliner::_IsInModelSelection(pxr::UsdPrim prim)
{
    vector<pxr::UsdPrim> sel = GetModel()->GetSelection();
    // check if prim in model selection
    return find(sel.begin(), sel.end(), prim) != sel.end();
}

void Outliner::_DrawChildrendHierarchyDecoration(ImRect parentRect,
                                                vector<ImRect> childrenRects)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImColor lineColor = ImGui::GetColorU32(ImGuiCol_Text, 0.25f);

    // all decoration for children will start at horizStartPos
    float horizStartPos = parentRect.Min.x + 10.0f;  // hard coded

    // save the lineStart of the last children to draw the vertical
    // line from the parent to the last child
    ImVec2 lineStart;

    // loop over all children and draw every horizontal line
    for (auto rect : childrenRects) {
        const float midpoint = (rect.Min.y + rect.Max.y) / 2.0f;
        const float lineSize = 8.0f;  // hard coded

        lineStart = ImVec2(horizStartPos, midpoint);
        ImVec2 lineEnd = lineStart + ImVec2(lineSize, 0);

        drawList->AddLine(lineStart, lineEnd, lineColor);
    }

    // draw the vertical line from the parent to the last child
    ImVec2 lineEnd = ImVec2(horizStartPos, parentRect.Max.y + 2.0f);
    drawList->AddLine(lineStart, lineEnd, lineColor);
}
