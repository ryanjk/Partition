#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>
#include <Utilities\frame_string.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <Component\transform_t.h>

#include <chrono>

#include <Application\MainLoop.inc>

// -- Uniform buffer data definitions ----

struct alignas(16) directional_light_t {
	pn::vec3f direction;
	float intensity;
};

struct alignas(16) wave_t {
	float A;
	float L;
	float w;
	float q;
	pn::vec2f d;
};

// ----- program uniforms ------
#define N_WAVES 1

pn::cbuffer<directional_light_t>	directional_light;
pn::cbuffer_array<wave_t, N_WAVES>	wave;

// --- wave instance data ----
pn::transform_t					wave_transform;
pn::vector<pn::mesh_buffer_t>	wave_mesh_buffer;
pn::shader_program_t			wave_program;

// ---- monkey instance data -----
pn::transform_t					monkey_transform;
pn::vector<pn::mesh_buffer_t>	monkey_mesh_buffer;

pn::dx_vertex_shader			basic_vs;
pn::input_layout_data_t			basic_input_layout;
pn::dx_shader_reflection		basic_vs_reflector;

pn::dx_pixel_shader				basic_ps;
pn::dx_shader_reflection		basic_ps_reflector;

// ---- misc --------
pn::linear_allocator frame_alloc(1024 * 1024);

// ---- misc d3d11 state -----
pn::dx_blend_state blend_state;

pn::string recomp() {
	auto vs_byte_code = pn::CompileVertexShader(pn::GetResourcePath("water.hlsl"));
	if (pn::Size(vs_byte_code) > 0) {
		wave_program.vertex_shader_data.shader		= pn::CreateVertexShader(device, vs_byte_code);
		wave_program.input_layout_data				= pn::CreateInputLayout(device, vs_byte_code);
		wave_program.vertex_shader_data.reflection	= pn::GetShaderReflector(vs_byte_code);
	}
	else {
		return "Compilation error, check log.";
	}

	auto ps_byte_code = pn::CompilePixelShader(pn::GetResourcePath("water.hlsl"));
	if (pn::Size(ps_byte_code) > 0) {
		wave_program.pixel_shader_data.shader		= pn::CreatePixelShader(device, ps_byte_code);
		wave_program.pixel_shader_data.reflection	= pn::GetShaderReflector(ps_byte_code);
	}
	else {
		return "Compilation error, check log.";
	}

	return "Successfully recompiled.";
}

void Init() {

	REGISTER_COMMAND(recomp, pn::string, void);

	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/");
	pn::SetResourceDirectoryName("Resources");

	// ---------- LOAD RESOURCES ----------------

	{
		auto mesh			= pn::LoadMesh(pn::GetResourcePath("water.fbx"));
		wave_mesh_buffer	= pn::CreateMeshBuffer(device, mesh);
	}

	{
		auto mesh			= pn::LoadMesh(pn::GetResourcePath("monkey.fbx"));
		monkey_mesh_buffer	= pn::CreateMeshBuffer(device, mesh);
	}

	// ------- SET BLENDING STATE ------------

	blend_state		= pn::CreateBlendState(device);
	pn::SetBlendState(device, blend_state);

	// --------- CREATE SHADER DATA ---------------

	{
		wave_program = pn::CompileShaderProgram(device, "water.hlsl");
	}

	{
		auto vs_byte_code	= pn::CompileVertexShader(pn::GetResourcePath("basic.hlsl"));
		basic_vs			= pn::CreateVertexShader(device, vs_byte_code);
		basic_input_layout	= pn::CreateInputLayout(device, vs_byte_code);
		basic_vs_reflector	= pn::GetShaderReflector(vs_byte_code);

		auto ps_byte_code	= pn::CompilePixelShader(pn::GetResourcePath("basic.hlsl"));
		basic_ps			= pn::CreatePixelShader(device, ps_byte_code);
		basic_ps_reflector	= pn::GetShaderReflector(ps_byte_code);
	}

	global_constants.data.screen_width	= static_cast<float>(pn::app::window_desc.width);
	global_constants.data.screen_height	= static_cast<float>(pn::app::window_desc.height);

	Initialize(device, directional_light);
	Initialize(device, wave);

	// init lights
	directional_light.data.direction = pn::vec3f(0.0f, 0.0f, 1.0f);
	directional_light.data.intensity = 1.0f;

	// init wave data
	for (int i = 0; i < N_WAVES; ++i) {
		wave.data[i].A = 0.615f;
		wave.data[i].L = 5.615;
		wave.data[i].w = 0.615;
		wave.data[i].q = 0;
		wave.data[i].d = { 0.68f, 0.735f };
	}

	// init camera
	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	camera_constants.data.proj = camera.GetMatrix();
	camera_constants.data.view = pn::mat4f::Identity;

	// init wave object
	wave_transform.position = { 0, 0, 15 };
	wave_transform.scale = { 1, 1, 1 };
	wave_transform.rotation = { 0.698f, 3.069f, 0.f };

	// init monkey object
	monkey_transform.position = { 0, 0, 15 };
	monkey_transform.scale = { 1,1,1 };
	monkey_transform.rotation = { 0,0,0 };

	// --------- INIT CUSTOM ALLOCATORS -----------
	pn::frame_string::SetFrameAllocator(&frame_alloc);
}

void Update(const float dt) {}

void Render() {
	auto context = pn::GetContext(device);

	// Update global uniforms
	global_constants.data.t				+= static_cast<float>(pn::app::dt);
	auto screen_desc					= pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
	global_constants.data.screen_width	= static_cast<float>(screen_desc.Width);
	global_constants.data.screen_height	= static_cast<float>(screen_desc.Height);

	// Set render target backbuffer color
	float color[] = { 0.0f, 0.0f, 0.0f, 1.000f };
	context->ClearRenderTargetView(render_target_view.Get(), color);
	context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// update directional light
	ImGui::Begin("Lights");

	ImGui::SliderFloat3("light dir", &directional_light.data.direction.x, -1.0f, 1.0f);
	ImGui::SliderFloat("light power", &directional_light.data.intensity, 0.0f, 10.0f);
	directional_light.data.direction = directional_light.data.direction == pn::vec3f::Zero ? pn::vec3f::Zero : pn::Normalize(directional_light.data.direction);

	ImGui::End(); // Lights

	SetProgramConstantBuffer(context, global_constants, wave_program);
	SetProgramConstantBuffer(context, camera_constants, wave_program);
	SetProgramConstantBuffer(context, model_constants, wave_program);
	SetProgramConstantBuffer(context, directional_light, wave_program);

	// update uniform buffers that are shared across shaders
	UpdateBuffer(context, global_constants);
	UpdateBuffer(context, camera_constants);
	UpdateBuffer(context, directional_light);


// ------ BEGIN WATER

	SetProgramConstantBuffer(context, wave, wave_program);

	pn::SetShaderProgram(context, wave_program);

	auto& wave_mesh = wave_mesh_buffer[0];
	pn::SetVertexBuffers(context, wave_program.input_layout_data, wave_mesh);

	// update wave
	ImGui::Begin("Waves");

	// update model matrix
	ImGui::SliderFloat3("position", &wave_transform.position.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("rotation", &wave_transform.rotation.x, -pn::TWOPI, pn::TWOPI);
	model_constants.data.model	= TransformToSRT(wave_transform);
	model_constants.data.mvp	= model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;

	for (int i = 0; i < N_WAVES; ++i) {
		pn::string w_id = pn::string("w") + std::to_string(i);
		ImGui::SliderFloat((w_id + " amp").c_str(), &wave.data[i].A, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " L").c_str(), &wave.data[i].L, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " w").c_str(), &wave.data[i].w, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " Q").c_str(), &wave.data[i].q, 0.0f, 3.0f);
		ImGui::SliderFloat2((w_id + " d").c_str(), &(wave.data[i].d.x), -1.0f, 1.0f);
		wave.data[i].d = (wave.data[i].d == pn::vec2f::Zero) ? pn::vec2f::Zero : pn::Normalize(wave.data[i].d);
	}

	ImGui::End(); // Waves

	// send updates to constant buffers
	UpdateBuffer(context, model_constants);
	UpdateBuffer(context, wave);

	pn::DrawIndexed(context, wave_mesh);

// ----- END WATER

// ----- BEGIN MONKEY

	/*pn::SetInputLayout(context, basic_input_layout);
	pn::SetVertexShader(context, basic_vs);
	pn::SetPixelShader(context, basic_ps);

	auto& monkey_mesh			= monkey_mesh_buffer[0];
	pn::SetVertexBuffers(context, basic_input_layout, monkey_mesh);

	model_constants.data.model	= TransformToSRT(monkey_transform);
	model_constants.data.mvp	= model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;

	UpdateBuffer(context, model_constants);

	pn::DrawIndexed(context, monkey_mesh); */

// --- END MONKEY

}

void MainLoopBegin() {
	frame_alloc.Release();
}

void MainLoopEnd() {}