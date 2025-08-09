#include "sceneindexview.h"

#include <ImGuizmo.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/imaging/hd/mergingSceneIndex.h>
#include "style/imgui_spectrum.h"
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

struct GraphEditorDelegate : public GraphEditor::Delegate {
    GraphEditorDelegate(Model* model):
        _model(model)
    {
        const ImGuiStyle& style = ImGui::GetStyle();
        ImVec4 color;

        color = style.Colors[ImGuiCol_Text];
        NODE_HEADER_COLOR = ImGui::ColorConvertFloat4ToU32(color);
        color = style.Colors[ImGuiCol_SeparatorHovered];
        NODE_BGD_COLOR = ImGui::ColorConvertFloat4ToU32(color);
        color = style.Colors[ImGuiCol_ScrollbarGrabActive];
        NODE_BGD_COLOR_HOVER = ImGui::ColorConvertFloat4ToU32(color);
    }

    bool AllowedLink(GraphEditor::NodeIndex from,
                        GraphEditor::NodeIndex to) override
    {
        // read only. AddLink and DelLink are void 
        return false;
    }

    void SelectNode(GraphEditor::NodeIndex nodeIndex,
                    bool selected) override
    {
        if(!selected)
            return;

        // allow only one node selected at most
        for(auto&& node: _nodes)
            node.selected = false;

        _nodes[nodeIndex].selected = selected;
        _model->SetActiveSceneIndex(_nodes[nodeIndex].sceneIndex);
    }

    void MoveSelectedNodes(const ImVec2 delta) override
    {
        for (auto& node : _nodes) {
            if (!node.selected) { continue; }
            node.x += delta.x;
            node.y += delta.y;
        }
    }

    virtual void RightClick(
        GraphEditor::NodeIndex nodeIndex,
        GraphEditor::SlotIndex slotIndexInput,
        GraphEditor::SlotIndex slotIndexOutput) override
    {
    }

    void AddLink(GraphEditor::NodeIndex inputNodeIndex,
                    GraphEditor::SlotIndex inputSlotIndex,
                    GraphEditor::NodeIndex outputNodeIndex,
                    GraphEditor::SlotIndex outputSlotIndex) override
    {
        // read only
    }

    void DelLink(GraphEditor::LinkIndex linkIndex) override
    {
        // read only
    }

    void CustomDraw(ImDrawList* drawList, ImRect rect,
                    GraphEditor::NodeIndex index) override
    {
        const auto& node = _nodes[index];
        // get the scene index type as description
        const string text = node.sceneIndexType;
        // center text in the drawable rect
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 pos;
        pos.x = rect.Min.x + (rect.GetWidth()  - textSize.x) * 0.5f;
        pos.y = rect.Min.y + (rect.GetHeight() - textSize.y) * 0.5f;

        drawList->AddText(pos, IM_COL32(0, 0, 0, 255), text.c_str());
    }

    const size_t GetTemplateCount() override
    {
        // max 10 inputs per Scene Index
        return 10;
    }

    const GraphEditor::Template GetTemplate(
        GraphEditor::TemplateIndex index) override
    {
        // template i contains i inputs
        return {
            NODE_HEADER_COLOR,
            NODE_BGD_COLOR,
            NODE_BGD_COLOR_HOVER,
            (ImU8)index,
            {},
            {},
            1,
            {},
            {},
        };
    }

    const size_t GetNodeCount() override
    {
        return _nodes.size();
    }

    const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
    {    
        const Node& node = _nodes[index];
        return GraphEditor::Node {
            node.sceneIndexName.c_str(),
            node.nbInputs,
            ImRect(
                ImVec2(node.x, node.y),
                ImVec2(node.x + node.width, node.y + node.height)
            ),
            node.selected
        };
    }

    const size_t GetLinkCount() override {
        return _links.size();
    }

    const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
    {
        return _links[index];
    }

    void SetTopSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
    {
        Clear();

        Node node = _CreateNode(sceneIndex);
        node.selected = true;
        _nodes.push_back(node);
        _ExploreChildrenFromNodeIdRecursive(0);
    }

    bool UpdateTopSceneIndex(HdSceneIndexBaseRefPtr sceneIndex)
    {
        // count number of scene indices. Only update if the 
        // number of scene indices is different from the number
        // of nodes.
        size_t nbSceneIndices = 0;
        std::vector<HdSceneIndexBaseRefPtr> siToProcess = {sceneIndex};
        do {
            auto si = siToProcess.back();
            siToProcess.pop_back();
            nbSceneIndices++;

            if(auto filterSi = TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(si))
                for(auto childSi: filterSi->GetInputScenes())
                    siToProcess.push_back(childSi);

        } while (!siToProcess.empty());
        
        // if number of scene indices different, reset the top scene index
        if(nbSceneIndices == _nodes.size())  
            return false;   
        
        SetTopSceneIndex(sceneIndex);
        return true;
    }

    void Clear()
    {
        _nodes.clear();
        _links.clear();
    }

    private:
        float NODE_MIN_WIDTH = 200;
        float NODE_HEIGHT = 40;
        float NODE_V_PADDING = 80;
        float NODE_H_PADDING = 100;
        ImU32 NODE_HEADER_COLOR;
        ImU32 NODE_BGD_COLOR;
        ImU32 NODE_BGD_COLOR_HOVER;

        Model* _model;

        struct Node {
                HdSceneIndexBaseRefPtr sceneIndex;
                string sceneIndexName;
                string sceneIndexType;
                size_t nbInputs;
                float x, y;
                float width, height;
                bool selected;
        };

        std::vector<Node> _nodes;
        std::vector<GraphEditor::Link> _links;

        Node _CreateNode(HdSceneIndexBaseRefPtr sceneIndex)
        {
            const string sceneIndexName = sceneIndex->GetDisplayName();
            const string sceneIndexType = _GetSceneIndexType(sceneIndex);
            ImVec2 typeSize = ImGui::CalcTextSize(sceneIndexType.c_str());
            ImVec2 nameSize = ImGui::CalcTextSize(sceneIndexName.c_str());
            float width = max(max(typeSize.x, nameSize.x) + 10, NODE_MIN_WIDTH);
            float height = NODE_HEIGHT;
            
            return {
                sceneIndex,
                sceneIndexName,
                sceneIndexType,
                0,
                0, 0,
                width, height,
                false
            };
        }

        string _GetSceneIndexType(HdSceneIndexBaseRefPtr sceneIndex)
        {
            if(TfDynamic_cast<HdMergingSceneIndexRefPtr>(sceneIndex))
                return "HdMergingSceneIndex";
            else if(TfDynamic_cast<HdSingleInputFilteringSceneIndexBaseRefPtr>(sceneIndex))
                return "HdSingleInputFilteringSceneIndexBase";
            else if(TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(sceneIndex))
                return "HdFilteringSceneIndexBase";
            else
                return "HdSceneIndexBase";
        }

        void _ExploreChildrenFromNodeIdRecursive(size_t nodeId)
        {
            const Node node = _nodes[nodeId];
            if(auto si = TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(node.sceneIndex)) {
                auto sceneIndices = si->GetInputScenes();
                size_t nbInputs = sceneIndices.size();
                _nodes[nodeId].nbInputs = nbInputs;
                float totalHeight = nbInputs * NODE_HEIGHT + (nbInputs - 1) * NODE_V_PADDING;

                for (size_t i = 0; i < nbInputs; i++) {
                    auto childSceneIndex = sceneIndices[i];

                    Node childNode = _CreateNode(childSceneIndex);
                    childNode.x = node.x - NODE_H_PADDING - childNode.width;
                    childNode.y = node.y - totalHeight / 2 + i * (NODE_V_PADDING + node.height) + node.height / 2;

                    size_t childNodeId = _nodes.size();
                    _nodes.push_back(childNode);
                    _links.push_back({childNodeId, 0, nodeId, i});

                    _ExploreChildrenFromNodeIdRecursive(childNodeId);
                }
            }
    }
};

SceneIndexView::SceneIndexView(Model* model, const string label) : View(model, label) {
    _fit = GraphEditor::Fit_None;
    delegate = new GraphEditorDelegate(model);

    const ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 color = style.Colors[ImGuiCol_HeaderActive];
    _options.mSelectedNodeBorderColor = ImGui::ColorConvertFloat4ToU32(color);
    _options.mLineThickness = 2;
    _options.mNodeSlotRadius = 6;
}

const string SceneIndexView::GetViewType()
{
    return VIEW_TYPE;
};

void SceneIndexView::_Draw()
{
    auto finalSceneIndex = GetModel()->GetFinalSceneIndex();
    bool topSceneIndexUpdated = delegate->UpdateTopSceneIndex(finalSceneIndex);
        
    if (ImGui::Button("Fit All Nodes")) {
        _fit = GraphEditor::Fit_AllNodes;
    }
    ImGui::SameLine();
    if (ImGui::Button("Fit Selected Nodes")) {
        _fit = GraphEditor::Fit_SelectedNodes;
    }
    ImGui::SameLine();
    if (ImGui::Button("Align Nodes Horizontally")) {
        delegate->SetTopSceneIndex(finalSceneIndex);
    }

    GraphEditor::Show(*delegate, _options, _viewState, true, &_fit);

    // Imguizmo seems to struggle to fit selection on first frame
    // if the view is small size. By doing so we force at least
    // one GraphEditor::Show with Fit_None. Then we fit selection
    if(topSceneIndexUpdated)
        _fit = GraphEditor::Fit_SelectedNodes;
}


PXR_NAMESPACE_CLOSE_SCOPE
