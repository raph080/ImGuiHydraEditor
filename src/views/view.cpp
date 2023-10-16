#include "view.h"

View::View(Model* model, const string label)
    : model(model),
      label(label){

      };

Model* View::GetModel()
{
    return model;
}

const string View::GetViewType()
{
    return VIEW_TYPE;
};

const string View::GetViewLabel()
{
    return label;
}
void View::Update()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::Begin(label.c_str(), &wasDisplayed, GetGizmoWindowFlags());

    // update focus state
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows |
                               ImGuiFocusedFlags_RootWindow)) {
        if (!wasFocused) {
            FocusInEvent();
            wasFocused = true;
        }
    }
    else if (wasFocused) {
        FocusOutEvent();
        wasFocused = false;
    }

    if (wasFocused) {
        int key = ImGuiKey_NamedKey_BEGIN;
        while (key < ImGuiKey_NamedKey_END) {
            if (ImGui::IsKeyPressed((ImGuiKey)key))
                KeyPressEvent((ImGuiKey)key);
            key++;
        }
    }

    Draw();

    // update inner rect size
    innerRect = ImGui::GetCurrentWindow()->InnerRect;

    if (ImGui::IsMouseHoveringRect(innerRect.Min, innerRect.Max)) {
        if (!wasHovered) {
            HoverInEvent();
            wasHovered = true;
        }
    }
    else if (wasHovered) {
        HoverOutEvent();
        wasHovered = false;
    }

    if (wasHovered) {
        // update mouse pos
        ImVec2 appMousePos = ImGui::GetMousePos();
        ImVec2 viewMousePos = appMousePos - innerRect.Min;

        MouseMoveEvent(_prevMousePos, viewMousePos);
        _prevMousePos = viewMousePos;

        for (int i = 0; i < 5; i++) {
            if (io.MouseClicked[i])
                MousePressEvent((ImGuiMouseButton_)i, viewMousePos);
            if (io.MouseReleased[i])
                MouseReleaseEvent((ImGuiMouseButton_)i, viewMousePos);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
};

bool View::IsDisplayed()
{
    return wasDisplayed;
}

void View::ModelChangedEvent(){};

ImRect View::GetInnerRect()
{
    return innerRect;
}

void View::Draw(){};

void View::FocusInEvent(){};

void View::FocusOutEvent(){};

void View::HoverInEvent(){};

void View::HoverOutEvent(){};

void View::KeyPressEvent(ImGuiKey key){};

void View::MousePressEvent(ImGuiMouseButton_ button, ImVec2 pos){};

void View::MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 pos){};

void View::MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos){};

ImGuiWindowFlags View::GetGizmoWindowFlags()
{
    return ImGuiWindowFlags_None;
};
