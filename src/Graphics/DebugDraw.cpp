#include <Graphics\DebugDraw.h>
#include <Graphics\DirectX.h>

#include <IO\PathUtil.h>

namespace pn::debug {

shader_program_t debug_draw_shader;

void InitDebugDraw() {
	debug_draw_shader = pn::CompileShaderProgram(pn::GetResourcePath("debug_draw.hlsl"));
}

void DrawLine(vec3f origin, vec3f direction, float length) {
	mesh_t line{};
	line.vertices = { origin, (length * direction) + origin };
	line.topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

	auto mesh_buffer = CreateMeshBuffer(line);

	SetShaderProgram(debug_draw_shader);
	SetVertexBuffers(mesh_buffer);


}

}