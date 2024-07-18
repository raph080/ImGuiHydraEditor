#include "view.h"

View::View(Model* model, const string label)
    : _model(model),
      _label(label),
      _wasFocused(false),
      _wasHovered(false),
      _wasDisplayed(true)
{
    _sceneIndex = GetModel()->GetFinalSceneIndex();
};

Model* View::GetModel()
{
    return _model;
}

const string View::GetViewType()
{
    return VIEW_TYPE;
};

const string View::GetViewLabel()
{
    return _label;
}
void View::Update()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::Begin(_label.c_str(), &_wasDisplayed, _GetGizmoWindowFlags());

    // update focus state
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows |
                               ImGuiFocusedFlags_RootWindow)) {
        if (!_wasFocused) {
            _FocusInEvent();
            _wasFocused = true;
        }
    }
    else if (_wasFocused) {
        _FocusOutEvent();
        _wasFocused = false;
    }

    if (_wasFocused) {
        int key = ImGuiKey_NamedKey_BEGIN;
        while (key < ImGuiKey_NamedKey_END) {
            if (ImGui::IsKeyPressed((ImGuiKey)key))
                _KeyPressEvent((ImGuiKey)key);
            key++;
        }
    }

    _Draw();

    // update inner rect size
    _innerRect = ImGui::GetCurrentWindow()->InnerRect;

    if (ImGui::IsMouseHoveringRect(_innerRect.Min, _innerRect.Max)) {
        if (!_wasHovered) {
            _HoverInEvent();
            _wasHovered = true;
        }
    }
    else if (_wasHovered) {
        _HoverOutEvent();
        _wasHovered = false;
    }

    if (_wasHovered) {
        // update mouse pos
        ImVec2 appMousePos = ImGui::GetMousePos();
        ImVec2 viewMousePos = appMousePos - _innerRect.Min;

        _MouseMoveEvent(_prevMousePos, viewMousePos);
        _prevMousePos = viewMousePos;

        for (int i = 0; i < 5; i++) {
            if (io.MouseClicked[i])
                _MousePressEvent((ImGuiMouseButton_)i, viewMousePos);
            if (io.MouseReleased[i])
                _MouseReleaseEvent((ImGuiMouseButton_)i, viewMousePos);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
};

bool View::IsDisplayed()
{
    return _wasDisplayed;
}

ImRect View::GetInnerRect()
{
    return _innerRect;
}

void View::_Draw() {};

void View::_FocusInEvent() {};

void View::_FocusOutEvent() {};

void View::_HoverInEvent() {};

void View::_HoverOutEvent() {};

void View::_KeyPressEvent(ImGuiKey key) {};

void View::_MousePressEvent(ImGuiMouseButton_ button, ImVec2 pos) {};

void View::_MouseReleaseEvent(ImGuiMouseButton_ button, ImVec2 pos) {};

void View::_MouseMoveEvent(ImVec2 prevPos, ImVec2 curPos) {};

ImGuiWindowFlags View::_GetGizmoWindowFlags()
{
    return ImGuiWindowFlags_None;
};
