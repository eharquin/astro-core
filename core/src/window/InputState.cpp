#include <core/window/InputState.hpp>

void InputState::reset()
{
    // Copy current states to previous states for next frame comparison
    for (int i = 0; i < keyCount; ++i) {
        _previousKeys[i] = _currentKeys[i];
    }
    for (int i = 0; i < buttonCount; ++i) {
        _previousButtons[i] = _currentButtons[i];
    }

    _lastCursorX = _cursorX;
    _lastCursorY = _cursorY;
    _scrollX = 0.0f;
    _scrollY = 0.0f;
}

void InputState::setKey(Key key, Action action)
{
    _currentKeys[static_cast<int>(key)] = action;
}

void InputState::setMouseButton(Button button, Action action)
{
    _currentButtons[static_cast<int>(button)] = action;
}

void InputState::setCursorPos(float x, float y)
{
    _cursorX = x;
    _cursorY = y;
}

void InputState::setScroll(float dx, float dy)
{
    _scrollX = dx;
    _scrollY = dy;
}

Action InputState::getKeyState(Key key) const
{
    return _currentKeys[static_cast<int>(key)];
}

Action InputState::getMouseButtonState(Button button) const
{
    return _currentButtons[static_cast<int>(button)];
}

bool InputState::isKeyDown(Key key) const
{
    return _currentKeys[static_cast<int>(key)] == Action::Press;
}

bool InputState::isKeyJustPressed(Key key) const
{
    return _currentKeys[static_cast<int>(key)] == Action::Press && _previousKeys[static_cast<int>(key)] == Action::Release;
}

bool InputState::isKeyJustReleased(Key key) const
{
    return _currentKeys[static_cast<int>(key)] == Action::Release && _previousKeys[static_cast<int>(key)] == Action::Press;
}

bool InputState::isMouseButtonDown(Button button) const
{
    return _currentButtons[static_cast<int>(button)] == Action::Press;
}

bool InputState::isMouseButtonJustPressed(Button button) const
{
    return _currentButtons[static_cast<int>(button)] == Action::Press && _previousButtons[static_cast<int>(button)] == Action::Release;
}

bool InputState::isMouseButtonJustReleased(Button button) const
{
    return _currentButtons[static_cast<int>(button)] == Action::Release && _previousButtons[static_cast<int>(button)] == Action::Press;
}

float InputState::cursorX() const { return _cursorX; }
float InputState::cursorY() const { return _cursorY; }
float InputState::cursorDeltaX() const { return _cursorX - _lastCursorX; }
float InputState::cursorDeltaY() const { return _cursorY - _lastCursorY; }

float InputState::scrollX() const { return _scrollX; }
float InputState::scrollY() const { return _scrollY; }
