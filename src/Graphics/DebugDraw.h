#pragma once

#include <Utilities\Math.h>

namespace pn {

namespace debug {

void InitDebugDraw();

void DrawLine(vec3f origin, vec3f direction, float length, vec3f color = { 1,1,1 });
void DrawLine(vec3f origin, vec3f direction, float length, vec4f color = { 1,1,1,1 });

}

}
