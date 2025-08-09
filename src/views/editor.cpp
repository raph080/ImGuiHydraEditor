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
}

SdfPath Editor::_GetPrimToDisplay()
{
    SdfPathVector primPaths = GetModel()->GetSelection();

    if (primPaths.size() > 0 && !primPaths[0].IsEmpty())
        _prevSelection = primPaths[0];

    // nothing selected, can we still display old selection?
    else if (!_sceneIndex->GetPrim(_prevSelection).dataSource)
        _prevSelection = SdfPath();

    return _prevSelection;
}

void Editor::_AppendDisplayColorAttr(SdfPath primPath)
{
    GfVec3f color = _colorFilterSceneIndex->GetDisplayColor(primPath);

    if (color == GfVec3f(-1.f)) return;

    // save the values before change
    GfVec3f prevColor = GfVec3f(color);

    float* data = color.data();
    if (ImGui::CollapsingHeader("Display Color", ImGuiTreeNodeFlags_DefaultOpen))
        ImGui::SliderFloat3("", data, 0, 1);

    // add opinion only if values change
    if (color != prevColor)
        _colorFilterSceneIndex->SetDisplayColor(primPath, color);
}

PXR_NAMESPACE_CLOSE_SCOPE
