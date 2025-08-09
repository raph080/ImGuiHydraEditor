#include "sceneindexattribute.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGuizmo.h>
#include <imgui.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>

#include <sstream>
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

SceneIndexAttribute::SceneIndexAttribute(Model* model, const string label) : View(model, label)
{
    auto textColorVec = ImGui::GetStyle().Colors[ImGuiCol_Text];
    INHERITED_ATTR_COL = ImGui::ColorConvertFloat4ToU32(textColorVec);
}

const string SceneIndexAttribute::GetViewType()
{
    return VIEW_TYPE;
};

void SceneIndexAttribute::_Draw()
{
    _DrawLegend();
    SdfPath primPath = _GetPrimToDisplay();
    if (!primPath.IsEmpty()) _AppendAllPrimAttrs(primPath);
}

void SceneIndexAttribute::_DrawLegend()
{
    if (ImGui::CollapsingHeader("Legend")) {
        auto textColorVec = ImGui::GetStyle().Colors[ImGuiCol_Text];
        auto newColorVec = ImGui::ColorConvertU32ToFloat4(NEW_ATTR_COL);
        auto modifiedColorVec = ImGui::ColorConvertU32ToFloat4(MODIFIED_ATTR_COL);

        if (ImGui::BeginTable("LegendTable", 3, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextColored(textColorVec, "Inherited attribute");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextColored(newColorVec, "New attribute");
            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(modifiedColorVec, "Modified value");
            ImGui::EndTable();
        }
    }
}

SdfPath SceneIndexAttribute::_GetPrimToDisplay()
{
    SdfPathVector primPaths = GetModel()->GetSelection();

    if (primPaths.size() > 0 && !primPaths[0].IsEmpty())
        _prevSelection = primPaths[0];

    // nothing selected, can we still display old selection?
    else if (!_sceneIndex->GetPrim(_prevSelection).dataSource)
        _prevSelection = SdfPath();

    return _prevSelection;
}

void SceneIndexAttribute::_AppendDataSourceAttrs(
    HdContainerDataSourceHandle containerDataSource,
    HdContainerDataSourceHandle prevContainerDataSource)
{
    auto names = containerDataSource->GetNames();

    // sort the names alphabetically
    std::sort(names.begin(), names.end(),
        [](const TfToken& a, const TfToken& b) {
            return a.GetString() < b.GetString();
        });

    for (auto&& token : names) {
        auto dataSource = containerDataSource->Get(token);

        HdDataSourceBaseHandle prevDataSource;
        if (prevContainerDataSource)
            prevDataSource = prevContainerDataSource->Get(token);

        const char* tokenText = token.GetText();
        auto color = _GetDataSourceColor(dataSource, prevDataSource);

        auto containerDataSource = HdContainerDataSource::Cast(dataSource);
        if (containerDataSource) {
            auto prevContainerDataSource = HdContainerDataSource::Cast(prevDataSource);

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            bool clicked = ImGui::TreeNodeEx(tokenText, ImGuiTreeNodeFlags_OpenOnArrow);
            ImGui::PopStyleColor();

            if (clicked) {    
                _AppendDataSourceAttrs(containerDataSource, prevContainerDataSource);
                ImGui::TreePop();
            }
        }

        auto sampledDataSource = HdSampledDataSource::Cast(dataSource);
        if (sampledDataSource) {
            auto prevSampledDataSource = HdSampledDataSource::Cast(prevDataSource);

            auto value = sampledDataSource->GetValue(0);
            std::stringstream ss;
            ss << value;
            
            ImGui::Columns(2);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%s", tokenText);
            ImGui::NextColumn();
            ImGui::BeginChild(tokenText, ImVec2(0, 14), false);
            ImGui::Text("%s", ss.str().c_str());
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::Columns();
        }
    }
}

ImU32 SceneIndexAttribute::_GetDataSourceColor(
    HdDataSourceBaseHandle dataSource,
    HdDataSourceBaseHandle prevDataSource)
{
    auto sampledDataSource = HdSampledDataSource::Cast(dataSource);
    if (sampledDataSource) {
        auto prevSampledDataSource = HdSampledDataSource::Cast(prevDataSource);
 
        if (!prevSampledDataSource) return NEW_ATTR_COL;

        auto value = sampledDataSource->GetValue(0);
        auto prevValue = prevSampledDataSource->GetValue(0);

        std::stringstream ss, prevSs;
        ss << value;
        prevSs << prevValue;

        if (ss.str() != prevSs.str()) return MODIFIED_ATTR_COL;
        return INHERITED_ATTR_COL;
    }

    auto containerDataSource = HdContainerDataSource::Cast(dataSource);

    if (!containerDataSource) {
        return INHERITED_ATTR_COL;
    }

    auto prevContainerDataSource = HdContainerDataSource::Cast(prevDataSource);

    auto finalColor = INHERITED_ATTR_COL;
    // if data container, loop over all data within container and check
    // if there are some change. The "most important" color wins.
    // most important: modified > new > inherit
    for (auto&& token : containerDataSource->GetNames()) {
        auto dataSource = containerDataSource->Get(token);

        HdDataSourceBaseHandle prevDataSource;
        if (prevContainerDataSource)
            prevDataSource = prevContainerDataSource->Get(token);

        auto color = _GetDataSourceColor(dataSource, prevDataSource);

        if (
            (color == MODIFIED_ATTR_COL) ||
            (finalColor == INHERITED_ATTR_COL && color == NEW_ATTR_COL)
        )
            finalColor = color;
    }
    
    return finalColor;
}

void SceneIndexAttribute::_AppendAllPrimAttrs(SdfPath primPath)
{
    HdSceneIndexPrim prim = _sceneIndex->GetPrim(primPath);
    TfTokenVector tokens = prim.dataSource->GetNames();

    if (tokens.size() < 1) return;

    // get handle to all container data sources of parent Scene Indices
    // in order to check if data in current container data source
    // is inherited, new or modified.
    HdContainerDataSourceHandle prevContainerDataSource;
    if(auto si = TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(_sceneIndex)) {
        auto sceneIndices = si->GetInputScenes();
        for( auto childSi : sceneIndices){
            HdSceneIndexPrim prim = childSi->GetPrim(primPath);
            if(prim.dataSource) prevContainerDataSource = prim.dataSource;
        }
    }

    _AppendDataSourceAttrs(prim.dataSource, prevContainerDataSource);
}

PXR_NAMESPACE_CLOSE_SCOPE
