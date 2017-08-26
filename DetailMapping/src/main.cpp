#include <Utilities\Profile.h>

#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>

#include <Application\ResourceDatabase.h>
#include <Application\MainLoop.inc>

struct alignas(16) directional_light_t {
	pn::vec3f direction;
	float intensity;
};

pn::cbuffer<directional_light_t>	directional_light;

pn::texture_t			normal_map;
pn::dx_sampler_state	ss;

pn::dx_blend_state blend_state;

// --- wave instance data ----
pn::transform_t			plane_transform;
pn::mesh_buffer_t		plane_mesh_buffer;
pn::shader_program_t	normal_map_program;

void Init() {
	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/");
	pn::SetResourceDirectoryName("Resources");

	// ------ LOAD MESH ----------

	pn::LoadMesh(pn::GetResourcePath("water.fbx"));
	plane_mesh_buffer = pn::rdb::GetMeshResource("Plane");

	// --------- LOAD TEXTURES -------------

	normal_map	= pn::LoadTexture2D(pn::GetResourcePath("pad_map.png"));
	ss			= pn::CreateSamplerState(device);

	// ------- SET BLENDING STATE ------------

	blend_state = pn::CreateBlendState(device);
	pn::SetBlendState(device, blend_state);

	// --------- LOAD SHADER -------------

	normal_map_program = pn::CompileShaderProgram(device, pn::GetResourcePath("normal_map.hlsl"));

	// -------- INIT PROGRAM STATE -------

	global_constants.data.screen_width = static_cast<float>(pn::app::window_desc.width);
	global_constants.data.screen_height = static_cast<float>(pn::app::window_desc.height);

	InitializeCBuffer(device, directional_light);

	// init lights
	directional_light.data.direction = pn::vec3f(0.0f, 0.0f, 1.0f);
	directional_light.data.intensity = 1.0f;

	// init camera
	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	camera_constants.data.proj = camera.GetMatrix();
	camera_constants.data.view = pn::mat4f::Identity;

	// init object
	plane_transform.position = { 0, 0, 15 };
	plane_transform.scale = { 1, 1, 1 };
	plane_transform.rotation = pn::EulerToQuaternion(0, 0, 0.f);
}

void Update(const float dt) {}

void Render() {
	//pn::StartProfile("Frame");

	auto context = pn::GetContext(device);

	// Update global uniforms
	global_constants.data.t += static_cast<float>(pn::app::dt);
	auto screen_desc = pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
	global_constants.data.screen_width = static_cast<float>(screen_desc.Width);
	global_constants.data.screen_height = static_cast<float>(screen_desc.Height);

	// Set render target backbuffer color
	float color[] = { 0.0f, 0.0f, 0.0f, 1.000f };
	context->ClearRenderTargetView(render_target_view.Get(), color);
	context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// update directional light
	ImGui::Begin("Lights");
	pn::quaternion light_q = pn::EulerToQuaternion(directional_light.data.direction);
	pn::gui::DragRotation("light direction", &light_q);
	directional_light.data.direction = pn::QuaternionToEuler(light_q);
	//pn::gui::DragFloat3("light dir", &directional_light.data.direction.x, -1.0f, 1.0f);
	pn::gui::DragFloat("light power", &directional_light.data.intensity, 0.0f, 10.0f);
	//directional_light.data.direction = directional_light.data.direction == pn::vec3f::Zero ? pn::vec3f::Zero : pn::Normalize(directional_light.data.direction);

	ImGui::End(); // Lights

	SetProgramConstantBuffer(context, global_constants, normal_map_program);
	SetProgramConstantBuffer(context, camera_constants, normal_map_program);
	SetProgramConstantBuffer(context, model_constants, normal_map_program);
	SetProgramConstantBuffer(context, directional_light, normal_map_program);

	// update uniform buffers that are shared across shaders
	UpdateBuffer(context, global_constants);
	UpdateBuffer(context, camera_constants);
	UpdateBuffer(context, directional_light);

	// --- RENDER PLANE --------

	pn::SetShaderProgram(context, normal_map_program);

	auto& plane_mesh = plane_mesh_buffer;
	pn::SetVertexBuffers(context, normal_map_program.input_layout_data, plane_mesh);

	// update wave
	ImGui::Begin("Plane");
	// update model matrix
	pn::gui::EditStruct(plane_transform);
	model_constants.data.model = LocalToWorldMatrix(plane_transform);
	model_constants.data.model_view_inverse_transpose = pn::Transpose(pn::Inverse(model_constants.data.model * camera_constants.data.view));
	model_constants.data.mvp = model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;
	ImGui::End(); // Plane

	SetProgramShaderResources(context, normal_map, normal_map_program);
	SetProgramSamplers(context, ss, normal_map_program);

	// send updates to constant buffers
	UpdateBuffer(context, model_constants);

	pn::DrawIndexed(context, plane_mesh);

	//pn::EndProfile();
}

void MainLoopBegin() {}

void MainLoopEnd() {}