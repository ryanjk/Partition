#include <UI\UIUtil.h>

int g_debugVariableCount = 0;

pn::string GetDefaultVariableName(int id) {
	pn::string s = pn::string("v") + std::to_string(id);
	return s;
}

template<>
void ImGuiCall(const pn::string& name, float* value, float min, float max) {
	ImGui::SliderFloat(name.c_str(), value, min, max);
}