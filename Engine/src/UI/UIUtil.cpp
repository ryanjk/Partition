#include <UI\UIUtil.h>

#include <Input\Input.h>

// -------- VARIABLES ---------

int g_debugVariableCount = 0;

const float NORMAL_DRAG_SPEED = 0.1f;
const float SLOW_DRAG_SPEED = 0.01f;


// ------ FUNCTIONS ----------

pn::string GetDefaultVariableName(int id) {
	pn::string s = pn::string("v") + std::to_string(id);
	return s;
}

template<>
void	ImGuiCall(const pn::string& name, float* value, float min, float max, ui::default_t s) {
	ImGui::SliderFloat(name.c_str(), value, min, max);
}

template<>
void	ImGuiCall<pn::mat4f*, float, ui::transform_t>(const pn::string & name, pn::mat4f * value, float min, float max, ui::transform_t s) {
	pn::vec4f translation, scale;
	pn::quaternion rotation;
	//Decompose(*value, translation, rotation, scale);

	ImGui::SliderFloat3((name + " position").c_str(), (float*) &translation, min, max);
	ImGui::SliderFloat3((name + " scale").c_str(), (float*) &scale, 0, max);
	ImGuiCall(name + " rotation", &rotation, -10.0f, 10.0f, ui::quaternion_t());
	//ImGui::SliderFloat4((name + " rotation").c_str(), (float*) &rotation, min, max);
	//*value = DirectX::XMMatrixScalingFromVector(scale) * DirectX::XMMatrixRotationQuaternion(rotation) * DirectX::XMMatrixTranslationFromVector(translation);
}

namespace pn {

namespace gui {

float GetDragSpeed() {
	float DRAG_SPEED = NORMAL_DRAG_SPEED;
	if (pn::input::GetKeyState(pn::input::input_key::CONTROL) == pn::input::key_state::PRESSED) DRAG_SPEED = SLOW_DRAG_SPEED;
	return DRAG_SPEED;
}

bool DragFloat(const char* label, float* v, float min, float max, float speed_modifer, const char* display_format, float power) {
	return ImGui::DragFloat(label, v, GetDragSpeed() * speed_modifer, min, max, display_format, power);
}
bool DragFloat2(const char* label, float* v, float min, float max, float speed_modifier, const char* display_format, float power) {
	return ImGui::DragFloat2(label, v, GetDragSpeed() * speed_modifier, min, max, display_format, power);
}
bool DragFloat3(const char* label, float* v, float min, float max, float speed_modifer, const char* display_format, float power) {
	return ImGui::DragFloat3(label, v, GetDragSpeed() * speed_modifer, min, max, display_format, power);
}
bool DragFloat4(const char* label, float* v, float min, float max, float speed_modifer, const char* display_format, float power) {
	return ImGui::DragFloat4(label, v, GetDragSpeed() * speed_modifer, min, max, display_format, power);
}

} // namespace gui


} // namespace pn
