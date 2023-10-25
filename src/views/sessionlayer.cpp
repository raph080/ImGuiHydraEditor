#include "sessionlayer.h"

SessionLayer::SessionLayer(Model* model, const string label)
    : View(model, label)
{
    editor.SetPalette(GetPalette());
    editor.SetLanguageDefinition(GetUsdLanguageDefinition());
    editor.SetShowWhitespaces(false);
}

const string SessionLayer::GetViewType()
{
    return VIEW_TYPE;
};
void SessionLayer::Draw()
{
    if (IsSessionLayerUpdated()) LoadSessionTextFromModel();
    editor.Render("TextEditor");
};

bool SessionLayer::IsSessionLayerUpdated()
{
    pxr::SdfLayerHandle sessionLayer = GetModel()->GetSessionLayer();
    string layerText;
    sessionLayer->ExportToString(&layerText);
    return lastLoadedText != layerText;
}

void SessionLayer::LoadSessionTextFromModel()
{
    pxr::SdfLayerHandle sessionLayer = GetModel()->GetSessionLayer();
    string layerText;
    sessionLayer->ExportToString(&layerText);
    editor.SetText(layerText);
    lastLoadedText = layerText;
}

void SessionLayer::SaveSessionTextToModel()
{
    string editedText = editor.GetText();
    pxr::SdfLayerHandle sessionLayer = GetModel()->GetSessionLayer();
    sessionLayer->ImportFromString(editedText.c_str());
}

TextEditor::Palette SessionLayer::GetPalette()
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

TextEditor::LanguageDefinition SessionLayer::GetUsdLanguageDefinition()
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

void SessionLayer::FocusInEvent()
{
    isEditing = true;
};
void SessionLayer::FocusOutEvent()
{
    isEditing = false;
    SaveSessionTextToModel();
};
