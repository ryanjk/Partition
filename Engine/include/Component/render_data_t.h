#pragma once

#include <Component\transform_t.h>

#include <Graphics\DirectX.h>

namespace pn {

struct render_data_t {
	mesh_buffer_t*		mesh;
	shader_program_t*	shader_program;
};

} // namespace pn