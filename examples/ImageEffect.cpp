#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>
#include <Graphics\RenderSystem.h>

#include <Utilities\Logging.h>
#include <Utilities\frame_string.h>
#include <Utilities\Profile.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>
#include <UI\EditStruct.h>

#include <Component\transform_t.h>

#include <chrono>

#include <Application\ResourceDatabase.h>
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

template<>
void pn::gui::EditStruct(wave_t& wave) {
	DragFloat("amplitude##" , &wave.A, 0.0f, 10.0f);
	DragFloat("speed##"     , &wave.L, 0.0f, 10.0f);
	DragFloat("wavelength##", &wave.w, 0.0f, 10.0f);
	DragFloat("steepness##" , &wave.q, 0.0f, 3.0f);
	DragFloat2("direction##", &(wave.d.x), -1.0f, 1.0f);
	wave.d = (wave.d == pn::vec2f::Zero) ? pn::vec2f::Zero : pn::Normalize(wave.d);
}

// ----- program uniforms ------
#define N_WAVES 4

pn::cbuffer<directional_light_t>	directional_light;
pn::cbuffer_array<wave_t, N_WAVES>	wave;

// --- wave instance data ----
pn::transform_t			wave_transform;
pn::mesh_buffer_t		wave_mesh_buffer;
pn::shader_program_t	wave_program;

// ----- texture data ---------
pn::dx_resource_view	tex;
pn::dx_sampler_state	ss;

struct alignas(16) blur_params_t {
	pn::vec2f dir;
	float sigma;
};
pn::cbuffer<blur_params_t> blur_params;

// --- image effect data ---

pn::dx_render_target_view offscreen_render_target;
pn::dx_resource_view      offscreen_texture;

pn::dx_render_target_view offscreen_render_target2;
pn::dx_resource_view      offscreen_texture2;

pn::shader_program_t      image_program;

void Init() {

	// ---------- LOAD RESOURCES ----------------

	pn::StartProfile("Loading all meshes");

	{
		pn::StartProfile("Loading wave mesh");
		auto mesh_handle = pn::LoadMesh(pn::GetResourcePath("water.fbx"));
		pn::EndProfile();

		wave_mesh_buffer = pn::rdb::GetMeshResource("Plane");
		
	}

	pn::EndProfile();

	// --------- LOAD TEXTURES -------------

	tex	= pn::LoadTexture2D(pn::GetResourcePath("image.png"));
	ss	= pn::CreateSamplerState();

	// --------- CREATE SHADER DATA ---------------

	wave_program	= pn::CompileShaderProgram(pn::GetResourcePath("water.hlsl"));

	global_constants.data.screen_width	= static_cast<float>(pn::app::window_desc.width);
	global_constants.data.screen_height	= static_cast<float>(pn::app::window_desc.height);

	InitializeCBuffer(directional_light);
	InitializeCBuffer(wave);

	// init lights
	directional_light.data.direction = pn::vec3f(0.0f, 0.0f, 1.0f);
	directional_light.data.intensity = 1.0f;

	// init wave data
	for (int i = 0; i < N_WAVES; ++i) {
		wave.data[i].A = 0.615f;
		wave.data[i].L = 5.615f;
		wave.data[i].w = 0.615f;
		wave.data[i].q = 0.0f;
		wave.data[i].d = { 0.68f, 0.735f };
	}

	// init wave object
	wave_transform.position	= { 0, 0, 15 };
	wave_transform.scale	= { 1, 1, 1 };
	wave_transform.rotation = pn::EulerToQuaternion( 0.698f, 3.069f, 0.f );

	// --------- CREATE SHADER DATA FOR IMAGE EFFECTS SHADER ---------------

	InitializeCBuffer(blur_params);
	blur_params.data.sigma = 1;

	image_program = pn::CompileShaderProgram(pn::GetResourcePath("image_effect.hlsl"));

	// ---- CREATE OFF-SCREEN RENDER TARGET -----

	auto back_buffer           = pn::GetSwapChainBuffer(SWAP_CHAIN);
	auto back_buffer_desc      = pn::GetDesc(back_buffer);
	back_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	{
		auto offscreen_render_texture = pn::CreateTexture2D(back_buffer_desc);
		offscreen_texture             = pn::CreateShaderResourceView(offscreen_render_texture);
		offscreen_render_target       = pn::CreateRenderTargetView(offscreen_render_texture);
	}

	{
		auto offscreen_render_texture = pn::CreateTexture2D(back_buffer_desc);
		offscreen_texture2            = pn::CreateShaderResourceView(offscreen_render_texture);
		offscreen_render_target2      = pn::CreateRenderTargetView(offscreen_render_texture);
	}
}

void Update(const double dt) {}

void Render() {

	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.000f };
	pn::ClearRenderTargetView(offscreen_render_target, color);
	pn::ClearRenderTargetView(offscreen_render_target2, color);

	// update directional light
	ImGui::Begin("Lights");
		pn::gui::DragFloat3("light dir", &directional_light.data.direction.x, -1.0f, 1.0f);
		pn::gui::DragFloat("light power", &directional_light.data.intensity, 0.0f, 10.0f);
		directional_light.data.direction = directional_light.data.direction == pn::vec3f::Zero ? pn::vec3f::Zero : pn::Normalize(directional_light.data.direction);
	ImGui::End(); // Lights

	UpdateBuffer(directional_light);

	// ------ BEGIN WATER

	pn::SetStandardShaderProgram(wave_program);

	SetProgramConstant("directional_light", directional_light);
	SetProgramConstant("wave", wave);

	auto& wave_mesh = wave_mesh_buffer;
	pn::SetVertexBuffers(wave_mesh);

	// update wave
	ImGui::Begin("Waves");
	// update model matrix
	pn::gui::EditStruct(wave_transform);
	UpdateModelConstantCBuffer(wave_transform);

	for (int i = 0; i < N_WAVES; ++i) {
		ImGui::PushID(i);
		pn::gui::EditStruct(wave.data[i]);
		ImGui::PopID();
	}
	ImGui::End(); // Waves
	UpdateBuffer(wave);

	pn::SetProgramResource("tex", tex);
	pn::SetProgramSampler("ss", ss);

	SetDepthTest(true);
	pn::SetRenderTarget(offscreen_render_target, DISPLAY_DEPTH_STENCIL);

	pn::DrawIndexed(wave_mesh);

	// ----- END WATER

	// ----- RENDER GAUSSIAN BLUR -----

	ImGui::Begin("Blur");
		ImGui::DragFloat("sigma", &blur_params.data.sigma, 1, 0, 100);
	ImGui::End();

	{
		SetStandardShaderProgram(image_program);
		SetProgramConstant("blur_params", blur_params);
		SetProgramSampler("ss", ss);
		SetDepthTest(false);

		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ----- RENDER GAUSSIAN BLUR DIR 1 -----

		pn::SetRenderTarget(offscreen_render_target2);
		
		pn::SetProgramResource("offscreen_texture", offscreen_texture);

		blur_params.data.dir = pn::vec2f(1, 0);
		UpdateBuffer(blur_params);

		_context->Draw(4, 0);

		// ----- RENDER GAUSSIAN BLUR DIR 2 -----

		pn::SetRenderTarget(DISPLAY_RENDER_TARGET);

		pn::SetProgramResource("offscreen_texture", offscreen_texture2);

		blur_params.data.dir = pn::vec2f(0, 1);
		UpdateBuffer(blur_params);

		_context->Draw(4, 0);

	}
}

void MainLoopBegin() {
	
}

void MainLoopEnd() {

}

void Close() {

}