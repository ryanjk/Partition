#pragma once

#include <imgui\imgui.h>
#include <imgui\imgui_impl_dx11.h>

#include <Utilities\UtilityTypes.h>
#include <Utilities\Math.h>

#include <Component\transform_t.h>

#include <type_traits>


// ----- FUNCTIONS -------------

namespace pn {

namespace gui {

void SetGUI(bool on);
bool IsGUIOn();

void Begin(const char* name);
void End();

void PushID(int id);
void PopID();

bool DragFloat(const char* label, float* v, float min, float max, float speed_modifier = 1.0f, const char* display_format = "%.3f", float power = 1.0f);
bool DragFloat2(const char* label, float* v, float min, float max, float speed_modifier = 1.0f, const char* display_format = "%.3f", float power = 1.0f);
bool DragFloat3(const char* label, float* v, float min, float max, float speed_modifier = 1.0f, const char* display_format = "%.3f", float power = 1.0f);
bool DragFloat4(const char* label, float* v, float min, float max, float speed_modifier = 1.0f, const char* display_format = "%.3f", float power = 1.0f);

void DragRotation(const char* label, quaternion* q, const vec3f x = vec3f::UnitX, const vec3f y = vec3f::UnitY, const vec3f z = vec3f::UnitZ, float speed_modifier = 1.0f);

} // namespace gui

} // namespace pn
