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
void	ImGuiCall(const pn::string& name, DirectX::XMVECTOR* value, float min, float max, ui::quaternion_t s) {
	float angle = 2 * acos(value->m128_f32[3]);
	ImGui::SliderFloat("angle", &angle, -6.28f, 6.28f);
	DirectX::XMVECTOR axis;
	if (
		abs(value->m128_f32[0]) < 0.0001 &&
		abs(value->m128_f32[1]) < 0.0001 &&
		abs(value->m128_f32[2]) < 0.0001) {
		DirectX::XMVECTOR temp_rotation = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		ImGui::SliderFloat3("axis", &temp_rotation.m128_f32[0], min, max);

		if (!(
			abs(temp_rotation.m128_f32[0]) < 0.0001 &&
			abs(temp_rotation.m128_f32[1]) < 0.0001 &&
			abs(temp_rotation.m128_f32[2]) < 0.0001
			)) {
			*value = DirectX::XMQuaternionRotationAxis(temp_rotation, angle);
		}
	}
	else {
		DirectX::XMQuaternionToAxisAngle(&axis, &angle, *value);
		ImGui::SliderFloat3("axis", &axis.m128_f32[0], min, max);
		*value = DirectX::XMQuaternionRotationAxis(axis, angle);
	}
}

template<>
void	ImGuiCall<pn::mat4f*, float, ui::transform_t>(const pn::string & name, pn::mat4f * value, float min, float max, ui::transform_t s) {
	DirectX::XMVECTOR translation, scale, rotation;
//	DirectX::XMMatrixDecompose(&scale, &rotation, &translation, *value);

	ImGui::SliderFloat3((name + " position").c_str(), (float*) &translation, min, max);
	ImGui::SliderFloat3((name + " scale").c_str(), (float*) &scale, 0, max);
	ImGuiCall(name + " rotation", &rotation, -10.0f, 10.0f, ui::quaternion_t());
	//ImGui::SliderFloat4((name + " rotation").c_str(), (float*) &rotation, min, max);
	//*value = DirectX::XMMatrixScalingFromVector(scale) * DirectX::XMMatrixRotationQuaternion(rotation) * DirectX::XMMatrixTranslationFromVector(translation);
}
