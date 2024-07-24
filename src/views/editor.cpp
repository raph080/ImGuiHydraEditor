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

Editor::Editor(Model* model, const string label) : View(model, label)
{
    auto editableSceneIndex = GetModel()->GetEditableSceneIndex();
    _colorFilterSceneIndex =
        pxr::ColorFilterSceneIndex::New(editableSceneIndex);
    GetModel()->SetEditableSceneIndex(_colorFilterSceneIndex);
}

const string Editor::GetViewType()
{
    return VIEW_TYPE;
};

void Editor::_Draw()
{
    pxr::SdfPath primPath = _GetPrimToDisplay();

    if (primPath.IsEmpty()) return;

    _AppendDisplayColorAttr(primPath);
}

pxr::SdfPath Editor::_GetPrimToDisplay()
{
    pxr::SdfPathVector primPaths = GetModel()->GetSelection();

    if (primPaths.size() > 0 && !primPaths[0].IsEmpty())
        _prevSelection = primPaths[0];

    return _prevSelection;
}

void Editor::_AppendDisplayColorAttr(pxr::SdfPath primPath)
{
    pxr::GfVec3f color = _colorFilterSceneIndex->GetDisplayColor(primPath);

    if (color == pxr::GfVec3f(-1.f)) return;

    // save the values before change
    pxr::GfVec3f prevColor = pxr::GfVec3f(color);

    float* data = color.data();
    if (ImGui::CollapsingHeader("Extra attributes"))
        ImGui::SliderFloat3("", data, 0, 1);

    // add opinion only if values change
    if (color != prevColor)
        _colorFilterSceneIndex->SetDisplayColor(primPath, color);
}
