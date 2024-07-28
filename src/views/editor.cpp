#include "editor.h"

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

PXR_NAMESPACE_OPEN_SCOPE

Editor::Editor(Model* model, const string label) : View(model, label)
{
    auto editableSceneIndex = GetModel()->GetEditableSceneIndex();
    _colorFilterSceneIndex =
        ColorFilterSceneIndex::New(editableSceneIndex);
    GetModel()->SetEditableSceneIndex(_colorFilterSceneIndex);
}

const string Editor::GetViewType()
{
    return VIEW_TYPE;
};

void Editor::_Draw()
{
    SdfPath primPath = _GetPrimToDisplay();

    if (primPath.IsEmpty()) return;

    _AppendDisplayColorAttr(primPath);
    _AppendAllPrimAttrs(primPath);
}

SdfPath Editor::_GetPrimToDisplay()
{
    SdfPathVector primPaths = GetModel()->GetSelection();

    if (primPaths.size() > 0 && !primPaths[0].IsEmpty())
        _prevSelection = primPaths[0];

    return _prevSelection;
}

void Editor::_AppendDisplayColorAttr(SdfPath primPath)
{
    GfVec3f color = _colorFilterSceneIndex->GetDisplayColor(primPath);

    if (color == GfVec3f(-1.f)) return;

    // save the values before change
    GfVec3f prevColor = GfVec3f(color);

    float* data = color.data();
    if (ImGui::CollapsingHeader("Display Color"))
        ImGui::SliderFloat3("", data, 0, 1);

    // add opinion only if values change
    if (color != prevColor)
        _colorFilterSceneIndex->SetDisplayColor(primPath, color);
}

void Editor::_AppendDataSourceAttrs(
    HdContainerDataSourceHandle containerDataSource)
{
    for (auto&& token : containerDataSource->GetNames()) {
        auto dataSource = containerDataSource->Get(token);
        const char* tokenText = token.GetText();

        auto containerDataSource =
            HdContainerDataSource::Cast(dataSource);
        if (containerDataSource) {
            bool clicked =
                ImGui::TreeNodeEx(tokenText, ImGuiTreeNodeFlags_OpenOnArrow);

            if (clicked) {
                _AppendDataSourceAttrs(containerDataSource);
                ImGui::TreePop();
            }
        }

        auto sampledDataSource = HdSampledDataSource::Cast(dataSource);
        if (sampledDataSource) {
            ImGui::Columns(2);
            VtValue value = sampledDataSource->GetValue(0);
            ImGui::Text("%s", tokenText);
            ImGui::NextColumn();
            ImGui::BeginChild(tokenText, ImVec2(0, 14), false);
            std::stringstream ss;
            ss << value;
            ImGui::Text("%s", ss.str().c_str());
            ImGui::EndChild();
            ImGui::Columns();
        }
    }
}

void Editor::_AppendAllPrimAttrs(SdfPath primPath)
{
    HdSceneIndexPrim prim = _sceneIndex->GetPrim(primPath);
    TfTokenVector tokens = prim.dataSource->GetNames();

    if (tokens.size() < 1) return;

    if (ImGui::CollapsingHeader("Prim attributes")) {
        _AppendDataSourceAttrs(prim.dataSource);
    }
}

PXR_NAMESPACE_CLOSE_SCOPE
