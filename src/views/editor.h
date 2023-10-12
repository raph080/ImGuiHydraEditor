#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <GraphEditor.h>
#include <ImGuizmo.h>
#include <boost/predef/os.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/imaging/cameraUtil/framing.h>
#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

#include <array>

#include "utils/usd.h"
#include "view.h"

using namespace std;

class Editor : public View {
    public:
        inline static const string VIEW_TYPE = "Editor";

        Editor(Model* model, const string label = VIEW_TYPE)
            : View(model, label)
        {
        }
        const string GetViewType() override { return VIEW_TYPE; };

    private:
        UsdPrim _prevSelection;

        void Draw() override
        {
            UsdPrim prim = GetPrimToDisplay();
            UsdStageRefPtr stage = GetModel()->GetStage();

            if (prim.IsValid()) {
                if (ImGui::CollapsingHeader("Transform attributes"))
                    AppendTransformAttrs(prim);

                if (prim.GetTypeName() == UsdGeomTokens->Camera)
                    if (ImGui::CollapsingHeader("Camera attributes"))
                        AppendCamAttrs(prim);

                if (prim.HasAttribute(UsdGeomTokens->primvarsDisplayColor))
                    if (ImGui::CollapsingHeader("Extra attributes"))
                        AppendDisplayColorAttr(prim);
            }
        }

        /**
         * @brief Get the Usd prim to display the attributes in the editor.
         * The Usd prim correspond to the selected one or the previous selected
         * if the selection is empty
         *
         * @return The Usd Prim to display the attributes
         */
        UsdPrim GetPrimToDisplay()
        {
            vector<UsdPrim> prims = GetModel()->GetSelection();

            if (prims.size() > 0 && prims[0].IsValid())
                _prevSelection = prims[0];

            return _prevSelection;
        }

        void AppendTransformAttrs(UsdPrim prim)
        {
            UsdGeomGprim gprim(prim);

            GfMatrix4d transform = GetTransform(gprim);
            GfMatrix4f transformF(transform);

            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(transformF.data(),
                                                  matrixTranslation,
                                                  matrixRotation, matrixScale);

            ImGui::InputFloat3("Translation", matrixTranslation);
            ImGui::InputFloat3("Rotation", matrixRotation);
            ImGui::InputFloat3("Scale", matrixScale);

            ImGuizmo::RecomposeMatrixFromComponents(
                matrixTranslation, matrixRotation, matrixScale,
                transformF.data());

            if (!AreNearlyEquals(transformF, GfMatrix4f(transform)))
                SetTransform(gprim, GfMatrix4d(transformF));
        }

        void AppendCamAttrs(UsdPrim prim)
        {
            UsdGeomCamera cam(prim);

            vector<TfToken> attTokens = {
                UsdGeomTokens->clippingRange,
                UsdGeomTokens->focalLength,
                UsdGeomTokens->horizontalAperture,
                UsdGeomTokens->horizontalApertureOffset,
                UsdGeomTokens->verticalAperture,
                UsdGeomTokens->verticalApertureOffset};

            for (UsdAttribute attr : prim.GetAttributes()) {
                if (attr.GetTypeName() == SdfValueTypeNames->Float) {
                    float value;
                    attr.Get(&value);
                    float oldValue(value);
                    ImGui::InputFloat(attr.GetName().GetText(), &value);
                    if (value != oldValue) attr.Set(value);
                }
                else if (attr.GetTypeName() == SdfValueTypeNames->Float2) {
                    GfVec2f value;
                    attr.Get(&value);
                    GfVec2f oldValue(value);
                    ImGui::InputFloat2(attr.GetName().GetText(), value.data());
                    if (value != oldValue) attr.Set(value);
                }
            }
        }

        void AppendDisplayColorAttr(UsdPrim prim)
        {
            // get the display color attr from UsdGeomGprim
            UsdGeomGprim gprim(prim);
            UsdAttribute attr = gprim.GetDisplayColorAttr();

            VtVec3fArray values;
            bool success = attr.Get(&values, UsdTimeCode::Default());
            // if impossible to read, set a default value of [0.5, 0.5, 0.5]
            if (!success) values = VtVec3fArray(1, GfVec3f(0.5f));

            // save the values before change
            VtVec3fArray prevValues = VtVec3fArray(values);

            float* data = values.data()[0].data();
            ImGui::SliderFloat3("Display Color", data, 0, 1);

            // add opinion only if values change
            if (values != prevValues) attr.Set(values, UsdTimeCode::Default());
        }
};