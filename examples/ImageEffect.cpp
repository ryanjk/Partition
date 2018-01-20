#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

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

// ---- misc d3d11 state -----
pn::dx_blend_state        ENABLE_ALPHA_BLENDING;

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
pn::dx_buffer             screen_mesh_buffer;
pn::dx_buffer             screen_index_buffer;

pn::dx_depth_stencil_state DISABLE_DEPTH_TEST;

void Init() {
	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/");
	pn::SetResourceDirectoryName("Resources");

	Log("Size of resource id: {} bytes", sizeof(pn::rdb::resource_id_t));

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

	tex				= pn::LoadTexture2D(pn::GetResourcePath("image.png"));
	ss	= pn::CreateSamplerState();

	// ------- SET BLENDING STATE ------------

	auto desc             = pn::CreateAlphaBlendDesc();
	ENABLE_ALPHA_BLENDING = pn::CreateBlendState(&desc);
	pn::SetBlendState(ENABLE_ALPHA_BLENDING);

	// --------- CREATE DEPTH STATE -----

	CD3D11_DEPTH_STENCIL_DESC ds_desc(CD3D11_DEFAULT{});
	ds_desc.DepthEnable = false;
	DISABLE_DEPTH_TEST  = pn::CreateDepthStencilState(&ds_desc);

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

	// init camera
	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	camera_constants.data.proj = camera.GetMatrix();
	camera_constants.data.view = pn::mat4f::Identity;

	// init wave object
	wave_transform.position	= { 0, 0, 15 };
	wave_transform.scale	= { 1, 1, 1 };
	wave_transform.rotation = pn::EulerToQuaternion( 0.698f, 3.069f, 0.f );

	// --------- CREATE SHADER DATA FOR IMAGE EFFECTS SHADER ---------------

	InitializeCBuffer(blur_params);
	blur_params.data.sigma = 1;

	image_program = pn::CompileShaderProgram(pn::GetResourcePath("image_effect.hlsl"));

	// ------- CREATE MESH BUFFER FOR SCREEN ---------

	{
		static const pn::vec3f vertices[4] = {
			pn::vec3f(-1,-1,0),
			pn::vec3f(-1,1,0),
			pn::vec3f(1,1,0),
			pn::vec3f(1,-1,0),
		};
		screen_mesh_buffer = pn::CreateVertexBuffer(vertices, 4);
	}

	{
		static const int indices[6] = {
			0, 1, 2,
			0, 2, 3
		};
		screen_index_buffer = pn::CreateIndexBuffer(indices, 6);
	}

	// ---- CREATE OFF-SCREEN RENDER TARGET -----

	auto back_buffer           = pn::GetSwapChainBuffer(swap_chain);
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

void Update(const float dt) {}

void Render() {
	auto context = pn::GetContext(device);

	// Set render target backbuffer color
	float color[] = { 0.0f, 0.0f, 0.0f, 1.000f };

	context->ClearDepthStencilView(display_depth_stencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	context->ClearRenderTargetView(display_render_target.Get(), color);
	context->ClearRenderTargetView(offscreen_render_target.Get(), color);
	context->ClearRenderTargetView(offscreen_render_target2.Get(), color);

	// update directional light
	ImGui::Begin("Lights");

	pn::gui::DragFloat3("light dir", &directional_light.data.direction.x, -1.0f, 1.0f);
	pn::gui::DragFloat("light power", &directional_light.data.intensity, 0.0f, 10.0f);
	directional_light.data.direction = directional_light.data.direction == pn::vec3f::Zero ? pn::vec3f::Zero : pn::Normalize(directional_light.data.direction);

	ImGui::End(); // Lights

	// update uniform buffers that are shared across shaders
	UpdateBuffer(global_constants);
	UpdateBuffer(camera_constants);
	UpdateBuffer(directional_light);


// ------ BEGIN WATER
	
	SetProgramConstant(wave_program, "global_constants" , global_constants);
	SetProgramConstant(wave_program, "camera_constants" , camera_constants);
	SetProgramConstant(wave_program, "model_constants"  , model_constants);
	SetProgramConstant(wave_program, "directional_light", directional_light);
	SetProgramConstant(wave_program, "wave"             , wave);

	pn::SetShaderProgram(wave_program);

	auto& wave_mesh = wave_mesh_buffer;
	pn::SetVertexBuffers(wave_program.input_layout_data, wave_mesh);

	// update wave
	ImGui::Begin("Waves");
	// update model matrix
	pn::gui::EditStruct(wave_transform);
	model_constants.data.model	= LocalToWorldMatrix(wave_transform);
	model_constants.data.model_view_inverse_transpose = pn::Transpose(pn::Inverse(model_constants.data.model * camera_constants.data.view));
	model_constants.data.mvp	= model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;

	for (int i = 0; i < N_WAVES; ++i) {
		ImGui::PushID(i);
		pn::gui::EditStruct(wave.data[i]);
		ImGui::PopID();
	}
	ImGui::End(); // Waves

	SetProgramResource(wave_program, "tex", tex);

	SetProgramSampler(wave_program, "ss", ss);

	// send updates to constant buffers
	UpdateBuffer(model_constants);
	UpdateBuffer(wave);

	pn::SetDepthStencilState();
	context->OMSetRenderTargets(1, offscreen_render_target.GetAddressOf(), display_depth_stencil.Get());
	pn::DrawIndexed(wave_mesh);

// ----- END WATER

	// ----- RENDER GAUSSIAN BLUR -----

	ImGui::Begin("Blur");
		ImGui::DragFloat("sigma", &blur_params.data.sigma, 1, 0, 100);
	ImGui::End();

	{
		context->OMSetRenderTargets(1, offscreen_render_target2.GetAddressOf(), nullptr);

		SetProgramConstant(image_program, "global_constants", global_constants.buffer);
		SetProgramConstant(image_program, "blur_params"     , blur_params.buffer);

		pn::SetShaderProgram(image_program);

		static const unsigned int strides[1] = { sizeof(pn::vec3f) };
		static const unsigned int offsets[1] = { 0 };
		context->IASetVertexBuffers(0, 1, screen_mesh_buffer.GetAddressOf(), strides, offsets);
		context->IASetIndexBuffer(screen_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		SetProgramSampler(image_program, "ss", ss);

		pn::SetDepthStencilState(DISABLE_DEPTH_TEST);

		// ----- RENDER GAUSSIAN BLUR DIR 1 -----

		SetProgramResource(image_program, "offscreen_texture", offscreen_texture);
	
		blur_params.data.dir = pn::vec2f(1, 0);
		UpdateBuffer(blur_params);

		context->DrawIndexed(6, 0, 0);
	}

	// ----- RENDER GAUSSIAN BLUR DIR 2 -----
	
	{
		context->OMSetRenderTargets(1, display_render_target.GetAddressOf(), nullptr);

		SetProgramResource(image_program, "offscreen_texture", offscreen_texture2);

		blur_params.data.dir = pn::vec2f(0, 1);
		UpdateBuffer(blur_params);

		context->DrawIndexed(6, 0, 0);
	}
}

void MainLoopBegin() {
	
}

void MainLoopEnd() {

}

void Close() {

}