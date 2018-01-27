#pragma once

namespace pn {

struct vec3f;

namespace debug {

void InitDebugDraw();

void DrawLine(vec3f origin, vec3f direction, float length);

}

}
