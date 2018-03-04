#pragma once

#include <Utilities\Math.h>

#include <UI\EditorUI.h>

namespace pn {

struct alignas(16) light_t {
	vec3f light_position; float p;
	vec3f light_color;    float p2;
	float light_intensity;
};

template<>
void pn::gui::EditStruct(light_t& light) {
	DragFloat3("position##", &light.light_position.x, -10, 10);
	DragFloat3("color##", &light.light_color.x, 0, 1);
	DragFloat("intensity##", &light.light_intensity, 0, 100);
}

struct alignas(16) environment_lighting_t {
	float environment_intensity;
};

template<>
void pn::gui::EditStruct(environment_lighting_t& light) {
	DragFloat("intensity##", &light.environment_intensity, 0, 100);
}

} // namespace pn