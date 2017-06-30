#include <UI\UIUtil.h>

int g_debugVariableCount = 0;

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
