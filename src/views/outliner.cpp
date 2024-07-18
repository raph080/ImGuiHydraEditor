#include "outliner.h"

#include <pxr/imaging/hd/sceneIndexPrimView.h>
#include <pxr/usd/usd/stage.h>

Outliner::Outliner(Model* model, const string label) : View(model, label) {}

const string Outliner::GetViewType()
{
    return VIEW_TYPE;
};

void Outliner::_Draw()
{
    pxr::SdfPathVector paths = GetModel()->GetSceneIndex()->GetChildPrimPaths(
        pxr::SdfPath::AbsoluteRootPath());
    for (auto primPath : paths) _DrawPrimHierarchy(primPath);
}

// returns the node's rectangle
ImRect Outliner::_DrawPrimHierarchy(pxr::SdfPath primPath)
{
    bool recurse = _DrawHierarchyNode(primPath);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        GetModel()->SetSelection({primPath});
    }

    const ImRect curItemRect =
        ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    if (recurse) {
        // draw all children and store their rect position
        vector<ImRect> rects;
        pxr::SdfPathVector primPaths =
            GetModel()->GetSceneIndex()->GetChildPrimPaths(primPath);

        for (auto child : primPaths) {
            ImRect childRect = _DrawPrimHierarchy(child.GetPrimPath());
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

ImGuiTreeNodeFlags Outliner::_ComputeDisplayFlags(pxr::SdfPath primPath)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

    // set the flag if leaf or not
    pxr::SdfPathVector primPaths =
        GetModel()->GetSceneIndex()->GetChildPrimPaths(primPath);

    if (primPaths.size() == 0) {
        flags |= ImGuiTreeNodeFlags_Leaf;
        flags |= ImGuiTreeNodeFlags_Bullet;
    }
    else flags = ImGuiTreeNodeFlags_OpenOnArrow;

    // if selected prim, set highlight flag
    bool isSelected = _IsInModelSelection(primPath);
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    return flags;
}

bool Outliner::_DrawHierarchyNode(pxr::SdfPath primPath)
{
    bool recurse = false;
    const char* primName = primPath.GetName().c_str();
    ImGuiTreeNodeFlags flags = _ComputeDisplayFlags(primPath);

    // print node in blue if parent of selection
    if (_IsParentOfModelSelection(primPath)) {
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

bool Outliner::IsParentOf(pxr::SdfPath primPath, pxr::SdfPath childPrimPath)
{
    return primPath.GetCommonPrefix(childPrimPath) == primPath;
}

bool Outliner::_IsParentOfModelSelection(pxr::SdfPath primPath)
{
    // check if primPath is parent of selection
    for (auto&& p : GetModel()->GetSelection())
        if (IsParentOf(primPath, p)) return true;

    return false;
}

bool Outliner::_IsInModelSelection(pxr::SdfPath primPath)
{
    pxr::SdfPathVector sel = GetModel()->GetSelection();
    // check if primPath in model selection
    return find(sel.begin(), sel.end(), primPath) != sel.end();
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
