#include "editor.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGuizmo.h>
#include <imgui.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>

Editor::Editor(Model* model, const string label) : View(model, label) {}

const string Editor::GetViewType()
{
    return VIEW_TYPE;
};

void Editor::_Draw()
{
    pxr::UsdPrim prim = _GetPrimToDisplay();

    if (prim.IsValid()) {
        if (ImGui::CollapsingHeader("Transform attributes"))
            _AppendTransformAttrs(prim);

        if (prim.GetTypeName() == pxr::UsdGeomTokens->Camera)
            if (ImGui::CollapsingHeader("Camera attributes"))
                _AppendCamAttrs(prim);

        if (prim.HasAttribute(pxr::UsdGeomTokens->primvarsDisplayColor))
            if (ImGui::CollapsingHeader("Extra attributes"))
                _AppendDisplayColorAttr(prim);
    }
}

pxr::UsdPrim Editor::_GetPrimToDisplay()
{
    vector<pxr::UsdPrim> prims = GetModel()->GetSelection();

    if (prims.size() > 0 && prims[0].IsValid()) _prevSelection = prims[0];

    return _prevSelection;
}

void Editor::_AppendTransformAttrs(pxr::UsdPrim prim)
{
    pxr::UsdGeomGprim gprim(prim);

    pxr::GfMatrix4d transform = GetTransformMatrix(gprim);
    pxr::GfMatrix4f transformF(transform);

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(transformF.data(), matrixTranslation,
                                          matrixRotation, matrixScale);

    ImGui::InputFloat3("Translation", matrixTranslation);
    ImGui::InputFloat3("Rotation", matrixRotation);
    ImGui::InputFloat3("Scale", matrixScale);

    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation,
                                            matrixScale, transformF.data());

    if (!AreNearlyEquals(transformF, pxr::GfMatrix4f(transform)))
        SetTransformMatrix(gprim, pxr::GfMatrix4d(transformF));
}

void Editor::_AppendCamAttrs(pxr::UsdPrim prim)
{
    pxr::UsdGeomCamera cam(prim);

    vector<pxr::TfToken> attTokens = {
        pxr::UsdGeomTokens->clippingRange,
        pxr::UsdGeomTokens->focalLength,
        pxr::UsdGeomTokens->horizontalAperture,
        pxr::UsdGeomTokens->horizontalApertureOffset,
        pxr::UsdGeomTokens->verticalAperture,
        pxr::UsdGeomTokens->verticalApertureOffset};

    for (pxr::UsdAttribute attr : prim.GetAttributes()) {
        if (attr.GetTypeName() == pxr::SdfValueTypeNames->Float) {
            float value;
            attr.Get(&value);
            float oldValue(value);
            ImGui::InputFloat(attr.GetName().GetText(), &value);
            if (value != oldValue) attr.Set(value);
        }
        else if (attr.GetTypeName() == pxr::SdfValueTypeNames->Float2) {
            pxr::GfVec2f value;
            attr.Get(&value);
            pxr::GfVec2f oldValue(value);
            ImGui::InputFloat2(attr.GetName().GetText(), value.data());
            if (value != oldValue) attr.Set(value);
        }
    }
}

void Editor::_AppendDisplayColorAttr(pxr::UsdPrim prim)
{
    // get the display color attr from pxr::UsdGeomGprim
    pxr::UsdGeomGprim gprim(prim);
    pxr::UsdAttribute attr = gprim.GetDisplayColorAttr();

    pxr::VtVec3fArray values;
    bool success = attr.Get(&values, pxr::UsdTimeCode::Default());
    // if impossible to read, set a default value of [0.5, 0.5, 0.5]
    if (!success) values = pxr::VtVec3fArray(1, pxr::GfVec3f(0.5f));

    // save the values before change
    pxr::VtVec3fArray prevValues = pxr::VtVec3fArray(values);

    float* data = values.data()[0].data();
    ImGui::SliderFloat3("Display Color", data, 0, 1);

    // add opinion only if values change
    if (values != prevValues) attr.Set(values, pxr::UsdTimeCode::Default());
}
