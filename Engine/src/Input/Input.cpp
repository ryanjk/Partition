#include <Input\Input.h>

#include <Graphics\Window.h>

#include <cassert>

#include <Utilities\Logging.h>

namespace pn {

namespace input {

// -------- VARIABLES ---------------

input_state_t	input_state;
pn::string		input_characters;

// ------- FUNCTIONS --------

void InitInput() {}

void SetKeyState(int vkey, bool state) {
	assert(vkey < 256);
	input_state.keys[vkey] = state;
}

bool GetKeyState(const int vkey) {
	assert(vkey < 256);
	return input_state.keys[vkey];
}

void SetLeftMouse(const bool state) {
	input_state.left_mouse = state;
}

void SetRightMouse(const bool state) {
	input_state.right_mouse = state;
}

void SetMiddleMouse(const bool state) {
	input_state.middle_mouse = state;
}

void SetMousePos(const mouse_pos_t mouse_pos) {
	input_state.mouse_pos = mouse_pos;
}

mouse_pos_t GetMousePos() {
	return input_state.mouse_pos;
}

void IncrementMouseWheel(const float amount) {
	input_state.mouse_wheel += amount;
	Log("Mouse wheel {}", input_state.mouse_wheel);
}

void AddInputCharacter(const char c) {
	Insert(input_characters, c);
}

void ClearInputCharacters() {
	Clear(input_characters);
}

string GetInputCharacters() {
	return input_characters;
}

} // namespace input

} // namespace pn