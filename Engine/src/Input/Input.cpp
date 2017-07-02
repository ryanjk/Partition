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

void InitInput() {
	unsigned char i = 0;
	do {
		auto& cur_key = input_state.keys[i];
		cur_key = key_state::RELEASED;
		++i;
	} while (i != 0);
	input_state.mouse_pos = { 0, 0 };
	input_state.mouse_wheel = 0.0f;
}

void InputOnEndOfFrame() {
	Clear(input_characters);

	unsigned char i = 0U;
	do {
		const auto cur_key = static_cast<input_key>(i);
		const auto cur_key_state = GetKeyState(cur_key);
		if (cur_key_state == key_state::JUST_RELEASED) {
			SetKeyState(cur_key, key_state::RELEASED);
		}
		else if (cur_key_state == key_state::JUST_PRESSED) {
			SetKeyState(cur_key, key_state::PRESSED);
		}
		++i;
	} while (i != 0);
}

void SetKeyState(int vkey, key_state state) {
	assert(vkey < 256);
	input_state.keys[vkey] = state;

	/*if (state == key_state::JUST_PRESSED) {
		Log("Key action {}: {}", vkey, "Just pressed");
	} else if (state == key_state::JUST_RELEASED) {
		Log("Key action {}: {}", vkey, "Just released");
	} else if (state == key_state::PRESSED) {
		Log("Key action {}: {}", vkey, "pressed");
	} else if (state == key_state::RELEASED) {
		Log("Key action {}: {}", vkey, "released");
	} */
}

key_state GetKeyState(const input_key vkey) {
	return input_state.keys[vkey];
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

const string& GetInputCharacters() {
	return input_characters;
}

} // namespace input

} // namespace pn