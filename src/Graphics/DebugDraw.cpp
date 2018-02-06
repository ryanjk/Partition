#include <Graphics\DebugDraw.h>
#include <Graphics\DirectX.h>
#include <Graphics\RenderSystem.h>

#include <IO\PathUtil.h>

namespace pn::debug {

struct debug_draw_options_t {
	vec4f color;
};

static shader_program_t              debug_draw_shader;
static cbuffer<debug_draw_options_t> options;

void InitDebugDraw() {
	debug_draw_shader = pn::CompileShaderProgram(pn::GetResourcePath("debug_draw.hlsl"));

	InitializeCBuffer(options);
}

void DrawLine(vec3f origin, vec3f direction, float length, vec3f color) {
	DrawLine(origin, direction, length, { color, 1 });
}

void DrawLine(vec3f origin, vec3f direction, float length, vec4f color) {
	mesh_t line{};
	line.vertices = { origin, (length * direction) + origin };
	line.topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

	auto line_mesh_buffer = CreateMeshBuffer(line);

	SetShaderProgram(debug_draw_shader);
	SetProgramConstant("global_constants", global_constants);
	SetProgramConstant("camera_constants", camera_constants);
	SetProgramConstant("options", options);

	options.data.color = color;
	UpdateBuffer(options);

	SetVertexBuffers(line_mesh_buffer);
	
	_context->Draw(2, 0);

	ClearVertexBuffers();
	ClearShaderProgram();
}

}