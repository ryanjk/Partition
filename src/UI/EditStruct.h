#pragma once

#include <imgui\imgui.h>

namespace pn::gui {

template<typename T>
void EditStruct(T& struct_t) {}

template<typename T>
void EditStruct(const pn::string& window_name, T& struct_t) {
	ImGui::Begin(CString(window_name));
	EditStruct(struct_t);
	ImGui::End();
}

}
