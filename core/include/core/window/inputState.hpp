#pragma once

#include "input.hpp"

class InputState
{
public:
	void reset();

	void setKey(Key key, Action action);
	void setMouseButton(Button button, Action action);
	void setCursorPos(float x, float y);
	void setScroll(float dx, float dy);

	Action getKeyState(Key key) const;
	Action getMouseButtonState(Button button) const;

	bool isKeyDown(Key key) const;
	bool isKeyJustPressed(Key key) const;
	bool isKeyJustReleased(Key key) const;

	bool isMouseButtonDown(Button button) const;
	bool isMouseButtonJustPressed(Button button) const;
	bool isMouseButtonJustReleased(Button button) const;

	float cursorX() const;
	float cursorY() const;
	float cursorDeltaX() const;
	float cursorDeltaY() const;

	float scrollX() const;
	float scrollY() const;

private:
	static const int keyCount = 512;  // Max key count
	static const int buttonCount = 3; // 3 mouse buttons

	Action _currentKeys[keyCount] = {Action::Release};      // Current state of keys
	Action _previousKeys[keyCount] = {Action::Release};     // Previous state of keys

	Action _currentButtons[buttonCount] = {Action::Release}; // Current state of mouse buttons
	Action _previousButtons[buttonCount] = {Action::Release}; // Previous state of mouse buttons

	float _cursorX = 0.0f, _cursorY = 0.0f;
	float _lastCursorX = 0.0f, _lastCursorY = 0.0f;

	float _scrollX = 0.0f, _scrollY = 0.0f;
};