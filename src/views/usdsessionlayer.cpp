#include "usdsessionlayer.h"

#include <ImGuiFileDialog.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/plane.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usdImaging/usdImaging/sceneIndices.h>

#include <fstream>
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

UsdSessionLayer::UsdSessionLayer(Model* model, const string label)
    : View(model, label), _isEditing(false)
{
    _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;

    _editor.SetPalette(_GetPalette());
    _editor.SetLanguageDefinition(_GetUsdLanguageDefinition());
    _editor.SetShowWhitespaces(false);

    UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    const UsdImagingSceneIndices sceneIndices =
        UsdImagingCreateSceneIndices(info);

    _stageSceneIndex = sceneIndices.stageSceneIndex;
    GetModel()->AddSceneIndexBase(sceneIndices.finalSceneIndex);

    _SetEmptyStage();
}

const string UsdSessionLayer::GetViewType()
{
    return VIEW_TYPE;
};

ImGuiWindowFlags UsdSessionLayer::_GetGizmoWindowFlags()
{
    return _gizmoWindowFlags;
};

void UsdSessionLayer::_Draw()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New scene")) { _SetEmptyStage(); }

            if (ImGui::MenuItem("Load ...")) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadFile", "Choose File", ".usd,.usdc,.usda,.usdz", ".");
            }

            if (ImGui::MenuItem("Export to ...")) {
                ImGuiFileDialog::Instance()->OpenDialog(
                    "ExportFile", "Choose File", ".usd,.usdc,.usda,.usdz",
                    ".");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Objects")) {
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::MenuItem("Camera"))
                    _CreatePrim(HdPrimTypeTokens->camera);
                ImGui::Separator();
                if (ImGui::MenuItem("Capsule"))
                    _CreatePrim(HdPrimTypeTokens->capsule);
                if (ImGui::MenuItem("Cone"))
                    _CreatePrim(HdPrimTypeTokens->cone);
                if (ImGui::MenuItem("Cube"))
                    _CreatePrim(HdPrimTypeTokens->cube);
                if (ImGui::MenuItem("Cylinder"))
                    _CreatePrim(HdPrimTypeTokens->cylinder);
                if (ImGui::MenuItem("Sphere"))
                    _CreatePrim(HdPrimTypeTokens->sphere);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (_IsUsdSessionLayerUpdated()) _LoadSessionTextFromModel();
    _editor.Render("TextEditor");

    if (ImGuiFileDialog::Instance()->Display("LoadFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _LoadUsdStage(filePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("ExportFile")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _stage->Export(filePath, false);
        }
        ImGuiFileDialog::Instance()->Close();
    }
};

void UsdSessionLayer::_SetEmptyStage()
{
    _stage = UsdStage::CreateInMemory();
    UsdGeomSetStageUpAxis(_stage, UsdGeomTokens->y);

    _rootLayer = _stage->GetRootLayer();
    _sessionLayer = _stage->GetSessionLayer();

    _stage->SetEditTarget(_sessionLayer);
    _stageSceneIndex->SetStage(_stage);
    _stageSceneIndex->SetTime(UsdTimeCode::Default());

    GetModel()->SetStage(_stage);
}

void UsdSessionLayer::_LoadUsdStage(const string usdFilePath)
{
    if (!ifstream(usdFilePath)) {
        cout << "Error: the file does not exist. Empty stage loaded." << endl;
        _SetEmptyStage();
        return;
    }

    _rootLayer = SdfLayer::FindOrOpen(usdFilePath);
    _sessionLayer = SdfLayer::CreateAnonymous();
    _stage = UsdStage::Open(_rootLayer, _sessionLayer);
    _stage->SetEditTarget(_sessionLayer);
    _stageSceneIndex->SetStage(_stage);
    _stageSceneIndex->SetTime(UsdTimeCode::Default());

    GetModel()->SetStage(_stage);
}

string UsdSessionLayer::_GetNextAvailableIndexedPath(string primPath)
{
    UsdPrim prim;
    int i = -1;
    string newPath;
    do {
        i++;
        if (i == 0) newPath = primPath;
        else newPath = primPath + to_string(i);
        prim = _stage->GetPrimAtPath(SdfPath(newPath));
    } while (prim.IsValid());
    return newPath;
}

void UsdSessionLayer::_CreatePrim(TfToken primType)
{
    string primPath = _GetNextAvailableIndexedPath("/" + primType.GetString());

    if (primType == HdPrimTypeTokens->camera) {
        auto cam = UsdGeomCamera::Define(_stage, SdfPath(primPath));
        cam.CreateFocalLengthAttr(VtValue(18.46f));
    }
    else {
        UsdGeomGprim prim;
        if (primType == HdPrimTypeTokens->capsule)
            prim = UsdGeomCapsule::Define(_stage, SdfPath(primPath));
        if (primType == HdPrimTypeTokens->cone)
            prim = UsdGeomCone::Define(_stage, SdfPath(primPath));
        if (primType == HdPrimTypeTokens->cube)
            prim = UsdGeomCube::Define(_stage, SdfPath(primPath));
        if (primType == HdPrimTypeTokens->cylinder)
            prim =
                UsdGeomCylinder::Define(_stage, SdfPath(primPath));
        if (primType == HdPrimTypeTokens->sphere)
            prim = UsdGeomSphere::Define(_stage, SdfPath(primPath));

        VtVec3fArray extent({{-1, -1, -1}, {1, 1, 1}});
        prim.CreateExtentAttr(VtValue(extent));

        VtVec3fArray color({{.5f, .5f, .5f}});
        prim.CreateDisplayColorAttr(VtValue(color));
    }

    _stageSceneIndex->ApplyPendingUpdates();
}

bool UsdSessionLayer::_IsUsdSessionLayerUpdated()
{
    string layerText;
    _sessionLayer->ExportToString(&layerText);
    return _lastLoadedText != layerText;
}

void UsdSessionLayer::_LoadSessionTextFromModel()
{
    string layerText;
    _sessionLayer->ExportToString(&layerText);
    _editor.SetText(layerText);
    _lastLoadedText = layerText;
}

void UsdSessionLayer::_SaveSessionTextToModel()
{
    string editedText = _editor.GetText();
    _sessionLayer->ImportFromString(editedText.c_str());
}

TextEditor::Palette UsdSessionLayer::_GetPalette()
{
    ImU32 normalTxtCol = ImGui::GetColorU32(ImGuiCol_Text, 1.f);
    ImU32 halfTxtCol = ImGui::GetColorU32(ImGuiCol_Text, .5f);
    ImU32 strongKeyCol = ImGui::GetColorU32(ImGuiCol_HeaderActive, 1.f);
    ImU32 lightKeyCol = ImGui::GetColorU32(ImGuiCol_HeaderActive, .2f);
    ImU32 transparentCol = IM_COL32_BLACK_TRANS;

    return {{
        normalTxtCol,    // None
        strongKeyCol,    // Keyword
        strongKeyCol,    // Number
        strongKeyCol,    // String
        strongKeyCol,    // Char literal
        normalTxtCol,    // Punctuation
        normalTxtCol,    // Preprocessor
        normalTxtCol,    // Identifier
        strongKeyCol,    // Known identifier
        normalTxtCol,    // Preproc identifier
        halfTxtCol,      // Comment (single line)
        halfTxtCol,      // Comment (multi line)
        transparentCol,  // Background
        normalTxtCol,    // Cursor
        lightKeyCol,     // Selection
        normalTxtCol,    // ErrorMarker
        normalTxtCol,    // Breakpoint
        normalTxtCol,    // Line number
        transparentCol,  // Current line fill
        transparentCol,  // Current line fill (inactive)
        transparentCol,  // Current line edge
    }};
}

TextEditor::LanguageDefinition UsdSessionLayer::_GetUsdLanguageDefinition()
{
    TextEditor::LanguageDefinition langDef =
        TextEditor::LanguageDefinition::C();

    const char* const descriptors[] = {"add",    "append", "prepend", "del",
                                       "delete", "custom", "uniform", "rel"};
    const char* const keywords[] = {"references",
                                    "payload",
                                    "defaultPrim",
                                    "doc",
                                    "subLayers",
                                    "specializes",
                                    "active",
                                    "assetInfo",
                                    "hidden",
                                    "kind",
                                    "inherits",
                                    "instanceable",
                                    "customData",
                                    "variant",
                                    "variants",
                                    "variantSets",
                                    "config",
                                    "connect",
                                    "default",
                                    "dictionary",
                                    "displayUnit",
                                    "nameChildren",
                                    "None",
                                    "offset",
                                    "permission",
                                    "prefixSubstitutions",
                                    "properties",
                                    "relocates",
                                    "reorder",
                                    "rootPrims",
                                    "scale",
                                    "suffixSubstitutions",
                                    "symmetryArguments",
                                    "symmetryFunction",
                                    "timeSamples"};

    const char* const dataTypes[] = {
        "bool",       "uchar",      "int",        "int64",      "uint",
        "uint64",     "int2",       "int3",       "int4",       "half",
        "half2",      "half3",      "half4",      "float",      "float2",
        "float3",     "float4",     "double",     "double2",    "double3",
        "double4",    "string",     "token",      "asset",      "matrix2d",
        "matrix3d",   "matrix4d",   "quatd",      "quatf",      "quath",
        "color3d",    "color3f",    "color3h",    "color4d",    "color4f",
        "color4h",    "normal3d",   "normal3f",   "normal3h",   "point3d",
        "point3f",    "point3h",    "vector3d",   "vector3f",   "vector3h",
        "frame4d",    "texCoord2d", "texCoord2f", "texCoord2h", "texCoord3d",
        "texCoord3f", "texCoord3h"};

    const char* const schemas[] = {
        "Xform", "Scope", "Shader", "Sphere",   "Subdiv",         "Camera",
        "Cube",  "Curve", "Mesh",   "Material", "PointInstancer", "Plane"};

    const char* const specifiers[] = {"def", "over", "class", "variantSet"};

    for (auto& k : keywords) langDef.mKeywords.insert(k);
    for (auto& k : dataTypes) langDef.mKeywords.insert(k);
    for (auto& k : schemas) langDef.mKeywords.insert(k);
    for (auto& k : descriptors) langDef.mKeywords.insert(k);

    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String));

    langDef.mTokenRegexStrings.push_back(
        std::make_pair<std::string, TextEditor::PaletteIndex>(
            "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?"
            "[fF]?",
            TextEditor::PaletteIndex::Number));

    langDef.mCommentStart = "/*";
    langDef.mCommentEnd = "*/";
    langDef.mSingleLineComment = "#";

    langDef.mCaseSensitive = true;
    langDef.mAutoIndentation = true;

    langDef.mName = "USD";

    return langDef;
}

void UsdSessionLayer::_FocusInEvent()
{
    _isEditing = true;
};
void UsdSessionLayer::_FocusOutEvent()
{
    _isEditing = false;
    _SaveSessionTextToModel();
    _stageSceneIndex->ApplyPendingUpdates();
};

PXR_NAMESPACE_CLOSE_SCOPE
