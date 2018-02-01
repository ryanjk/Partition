#include <Utilities\Profile.h>

#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>

#include <Application\ResourceDatabase.h>
#include <Application\MainLoop.inc>

struct alignas(16) directional_light_t {
	pn::vec3f direction;
	float intensity;
};

struct alignas(16) mapping_vars_t {
	float height_scale;
	float height_offset;
};

pn::cbuffer<directional_light_t>	directional_light;
pn::cbuffer<mapping_vars_t>			mapping_vars;

pn::dx_resource_view		diffuse_map;
pn::dx_resource_view		normal_map;
pn::dx_resource_view		height_map;
pn::dx_sampler_state		ss;

// --- wave instance data ----
pn::transform_t			plane_transform;
pn::mesh_buffer_t		plane_mesh_buffer;
pn::shader_program_t	normal_map_program;

void Init() {

	// ------ LOAD MESH ----------

	pn::LoadMesh(pn::GetResourcePath("plane.fbx"));
	plane_mesh_buffer = pn::rdb::GetMeshResource("Plane");

	// --------- LOAD TEXTURES -------------

	diffuse_map = pn::LoadTexture2D(pn::GetResourcePath("diffuse.png"));
	normal_map	= pn::LoadTexture2D(pn::GetResourcePath("normal.png"));
	height_map	= pn::LoadTexture2D(pn::GetResourcePath("depth.png"));

	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	ss	= pn::CreateSamplerState(sampler_desc);

	// --------- LOAD SHADER -------------

	normal_map_program = pn::CompileShaderProgram(pn::GetResourcePath("normal_map.hlsl"));

	// -------- INIT PROGRAM STATE -------

	InitializeCBuffer(directional_light);

	// init lights
	directional_light.data.direction = pn::vec3f(0.0f, 0.0f, 1.0f);
	directional_light.data.intensity = 1.0f;

	// init object
	plane_transform.position = { 0, 0, 4 };
	plane_transform.scale = { 1, 1, 1 };
	plane_transform.rotation = pn::EulerToQuaternion(0, 0, 0.f);

	// init other cbuffers
	InitializeCBuffer(mapping_vars);
	mapping_vars.data.height_scale = 0.072f;
	mapping_vars.data.height_offset = 0.0f;
}

void Update(const double dt) {}

void Render() {

	// update directional light
	ImGui::Begin("Lights");
		pn::quaternion light_q = pn::EulerToQuaternion(directional_light.data.direction);
		pn::gui::DragRotation("light direction", &light_q);
		directional_light.data.direction = pn::QuaternionToEuler(light_q);
		pn::gui::DragFloat("light power", &directional_light.data.intensity, 0.0f, 10.0f);
	ImGui::End(); // Lights

	ImGui::Begin("Mapping Variables");
		pn::gui::DragFloat("height scale", &mapping_vars.data.height_scale, 0.0f, 0.15f, 0.01f);
		pn::gui::DragFloat("height offset", &mapping_vars.data.height_offset, -1.0f, 1.0f, 0.1f);
	ImGui::End();

	// --- RENDER PLANE --------
	
	pn::SetStandardShaderProgram(normal_map_program);
	
	pn::SetProgramConstant("directional_light", directional_light);
	pn::SetProgramConstant("mapping_vars"     , mapping_vars);

	// update uniform buffers that are shared across shaders
	UpdateBuffer(directional_light);
	UpdateBuffer(mapping_vars);

	pn::SetVertexBuffers(plane_mesh_buffer);

	// update wave
	ImGui::Begin("Plane");
		pn::gui::EditStruct(plane_transform);
	ImGui::End(); // Plane
	UpdateModelConstantCBuffer(plane_transform);

	pn::SetProgramResource("diffuse_map", diffuse_map);
	pn::SetProgramResource("normal_map", normal_map);
	pn::SetProgramResource("height_map", height_map);
	
	pn::SetProgramSampler("ss", ss);

	pn::DrawIndexed(plane_mesh_buffer);
}

void MainLoopBegin() {}

void MainLoopEnd() {}

void Close() {}