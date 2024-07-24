#include "usdsessionlayer.h"

#include <ImGuiFileDialog.h>
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

#include "pxr/imaging/hd/tokens.h"

UsdSessionLayer::UsdSessionLayer(Model* model, const string label)
    : View(model, label), _isEditing(false)
{
    _gizmoWindowFlags = ImGuiWindowFlags_MenuBar;

    _editor.SetPalette(_GetPalette());
    _editor.SetLanguageDefinition(_GetUsdLanguageDefinition());
    _editor.SetShowWhitespaces(false);

    pxr::UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    const pxr::UsdImagingSceneIndices sceneIndices =
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
                    _CreatePrim(pxr::HdPrimTypeTokens->camera);
                ImGui::Separator();
                if (ImGui::MenuItem("Capsule"))
                    _CreatePrim(pxr::HdPrimTypeTokens->capsule);
                if (ImGui::MenuItem("Cone"))
                    _CreatePrim(pxr::HdPrimTypeTokens->cone);
                if (ImGui::MenuItem("Cube"))
                    _CreatePrim(pxr::HdPrimTypeTokens->cube);
                if (ImGui::MenuItem("Cylinder"))
                    _CreatePrim(pxr::HdPrimTypeTokens->cylinder);
                if (ImGui::MenuItem("Sphere"))
                    _CreatePrim(pxr::HdPrimTypeTokens->sphere);
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
    _stage = pxr::UsdStage::CreateInMemory();
    UsdGeomSetStageUpAxis(_stage, pxr::UsdGeomTokens->y);

    _rootLayer = _stage->GetRootLayer();
    _sessionLayer = _stage->GetSessionLayer();

    _stage->SetEditTarget(_sessionLayer);
    _stageSceneIndex->SetStage(_stage);
    _stageSceneIndex->SetTime(pxr::UsdTimeCode::Default());

    GetModel()->SetStage(_stage);
}

void UsdSessionLayer::_LoadUsdStage(const string usdFilePath)
{
    if (!ifstream(usdFilePath)) {
        cout << "Error: the file does not exist. Empty stage loaded." << endl;
        _SetEmptyStage();
        return;
    }

    _rootLayer = pxr::SdfLayer::FindOrOpen(usdFilePath);
    _sessionLayer = pxr::SdfLayer::CreateAnonymous();
    _stage = pxr::UsdStage::Open(_rootLayer, _sessionLayer);
    _stage->SetEditTarget(_sessionLayer);
    _stageSceneIndex->SetStage(_stage);
    _stageSceneIndex->SetTime(pxr::UsdTimeCode::Default());

    GetModel()->SetStage(_stage);
}

string UsdSessionLayer::_GetNextAvailableIndexedPath(string primPath)
{
    pxr::UsdPrim prim;
    int i = -1;
    string newPath;
    do {
        i++;
        if (i == 0) newPath = primPath;
        else newPath = primPath + to_string(i);
        prim = _stage->GetPrimAtPath(pxr::SdfPath(newPath));
    } while (prim.IsValid());
    return newPath;
}

void UsdSessionLayer::_CreatePrim(pxr::TfToken primType)
{
    string primPath = _GetNextAvailableIndexedPath("/" + primType.GetString());

    if (primType == pxr::HdPrimTypeTokens->camera) {
        auto cam = pxr::UsdGeomCamera::Define(_stage, pxr::SdfPath(primPath));
        cam.CreateFocalLengthAttr(pxr::VtValue(18.46f));
    }
    else {
        pxr::UsdGeomGprim prim;
        if (primType == pxr::HdPrimTypeTokens->capsule)
            prim = pxr::UsdGeomCapsule::Define(_stage, pxr::SdfPath(primPath));
        if (primType == pxr::HdPrimTypeTokens->cone)
            prim = pxr::UsdGeomCone::Define(_stage, pxr::SdfPath(primPath));
        if (primType == pxr::HdPrimTypeTokens->cube)
            prim = pxr::UsdGeomCube::Define(_stage, pxr::SdfPath(primPath));
        if (primType == pxr::HdPrimTypeTokens->cylinder)
            prim =
                pxr::UsdGeomCylinder::Define(_stage, pxr::SdfPath(primPath));
        if (primType == pxr::HdPrimTypeTokens->sphere)
            prim = pxr::UsdGeomSphere::Define(_stage, pxr::SdfPath(primPath));

        pxr::VtVec3fArray extent({{-1, -1, -1}, {1, 1, 1}});
        prim.CreateExtentAttr(pxr::VtValue(extent));

        pxr::VtVec3fArray color({{.5f, .5f, .5f}});
        prim.CreateDisplayColorAttr(pxr::VtValue(color));
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
