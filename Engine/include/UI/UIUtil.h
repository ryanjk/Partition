#pragma once

#include <imgui\imgui.h>
#include <imgui\imgui_impl_dx11.h>

#include <Utilities\UtilityTypes.h>

#include <type_traits>

#define VDBM(value, min, max) VDB(#value, value, min, max)

extern int g_debugVariableCount;

pn::string	GetDefaultVariableName(int id);

template<typename T>
void		ImGuiCall(const pn::string& name, T value, typename std::remove_pointer<T>::type min, typename std::remove_pointer<T>::type max) {}

template<>
void		ImGuiCall(const pn::string& name, float* value, float min, float max);

template<typename T>
void		VDB(const pn::string& name, T value, typename std::remove_pointer<T>::type min, typename std::remove_pointer<T>::type max) {
	g_debugVariableCount += 1;
	auto variable_name = name;
	if (name.empty()) {
		variable_name = GetDefaultVariableName(g_debugVariableCount);
	}
	ImGuiCall(variable_name, value, min, max);
}