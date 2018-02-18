#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>
#include <Graphics\RenderSystem.h>
#include <Graphics\GBuffer.h>

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

#include <System\Flycam.h>

using namespace pn;

// --- cbuffer ---

struct alignas(16) light_t {
	vec3f light_position; float p;
	vec3f light_color;    float p2;
	float light_intensity;
};

template<>
void pn::gui::EditStruct(light_t& light) {
	DragFloat3("position##", &light.light_position.x, -10, 10);
	DragFloat3("color##", &light.light_color.x, 0, 1);
	DragFloat("intensity##", &light.light_intensity, 0, 100);
}

#define NUM_LIGHTS 1
light_t lights[NUM_LIGHTS];
cbuffer<light_t> light;

struct alignas(16) environment_lighting_t {
	float environment_intensity;
};

template<>
void pn::gui::EditStruct(environment_lighting_t& light) {
	DragFloat("intensity##", &light.environment_intensity, 0, 100);
}

cbuffer<environment_lighting_t> environment_lighting;

// ---------------------------------

struct alignas(16) material_t {
	vec4f albedoMetal;
	vec4f specRoughness;
};

template<>
void gui::EditStruct(material_t& material) {
	DragFloat3("albedo", &material.albedoMetal.x, 0, 1);
	DragFloat3("specular", &material.specRoughness.x, 0, 1);
	DragFloat("roughness", &material.specRoughness.w, 0, 1, 0.2f);
	DragFloat("metallic", &material.albedoMetal.w, 0, 1, 0.2f);
}

cbuffer<material_t> material;

// --- gbuffer data ---

gbuffer_t albedo;
gbuffer_t world;
gbuffer_t normal;
gbuffer_t specular;

pn::shader_program_t gbuffer_fill_shader;
pn::shader_program_t simple_texture_shader;
pn::shader_program_t deferred_lighting_shader;
pn::shader_program_t deferred_env_lighting_shader;
pn::shader_program_t render_cubemap;

rdb::mesh_resource_t scene_mesh;

// Dragon data
transform_t dragon_transform;
dx_resource_view dragon_albedo;
dx_resource_view dragon_rough;

pn::dx_sampler_state ss;
dx_blend_state additive_blend;

// Environment map
transform_t      cubemap_transform;
dx_resource_view cubemap;
mesh_buffer_t    cubemap_mesh_buffer;

transform_t sphere_body_transform;
mesh_buffer_t sphere_body_mesh;

transform_t sphere_face_transform;
mesh_buffer_t sphere_face_mesh;

void Init() {

	// ---------- LOAD RESOURCES ----------------

	auto mesh_handle = pn::LoadMesh(pn::GetResourcePath("dragon.fbx"));
	scene_mesh = pn::rdb::GetMeshResource("default");
	dragon_transform.position = vec3f(0.0f, -4.0f, 9.0f);

	dragon_albedo = LoadTexture2D(GetResourcePath("AlbedoMetal.png"));
	dragon_rough  = LoadTexture2D(GetResourcePath("SomethingRough.png"));

	cubemap = LoadCubemap(GetResourcePath("space-cubemap.dds"));
	LoadMesh(GetResourcePath("cubemap.fbx"));
	cubemap_mesh_buffer = rdb::GetMeshResource("Cubemap");

	LoadMesh(pn::GetResourcePath("reflection_sphere.fbx"));
	sphere_body_mesh = rdb::GetMeshResource("SphereBody");
	sphere_body_transform.position = { 0,0,3 };

	sphere_face_mesh = rdb::GetMeshResource("SphereFace");
	sphere_face_transform.position = { 0,0,3 };

	LoadMesh(pn::GetResourcePath("round_sphere.fbx"));
	//sphere_body_mesh = rdb::GetMeshResource("RoundSphere");


	// --------- CREATE SHADER DATA ---------------

	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	ss = pn::CreateSamplerState(sampler_desc);

	gbuffer_fill_shader      = pn::CompileShaderProgram(pn::GetResourcePath("gbuffer_fill.hlsl"));
	simple_texture_shader    = pn::CompileShaderProgram(pn::GetResourcePath("simple_texture.hlsl"));
	deferred_lighting_shader = pn::CompileShaderProgram(pn::GetResourcePath("deferred_lighting.hlsl"));
	deferred_env_lighting_shader = pn::CompileShaderProgram(pn::GetResourcePath("deferred_env_lighting.hlsl"));
	render_cubemap = CompileShaderProgram(GetResourcePath("render_cubemap.hlsl"));

	// ---- CREATE GBUFFERS -----

	auto back_buffer           = pn::GetSwapChainBuffer(SWAP_CHAIN);
	auto back_buffer_desc      = pn::GetDesc(back_buffer);
	back_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	
	back_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	InitGBuffer(albedo  , back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R32_FLOAT;
	InitGBuffer(world   , back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	InitGBuffer(normal  , back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	InitGBuffer(specular, back_buffer_desc);

	// ----- INITIALIZE LIGHT DATA -----

	InitializeCBuffer(light);
	
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		lights[i].light_color     = vec3f(1, 1, 1);
		lights[i].light_intensity = 50;
		lights[i].light_position  = vec3f(0, 0.5f, 0);
	}

	InitializeCBuffer(environment_lighting);
	environment_lighting.data.environment_intensity = 1.0f;

	// ----- INIT MATERIAL ------

	InitializeCBuffer(material);
	material.data.albedoMetal   = vec4f(0.5, 0.5, 0.5, 0.5);
	material.data.specRoughness = vec4f(0.5, 0.5, 0.5, 0.5);

	// ----- CREATE BLEND DESC -------

	CD3D11_BLEND_DESC blend_desc(D3D11_DEFAULT);
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend    = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend   = D3D11_BLEND_ONE;
	additive_blend = CreateBlendState(&blend_desc);
}

void Resize() {
	CD3D11_TEXTURE2D_DESC desc = GetDesc(GetSwapChainBuffer(SWAP_CHAIN));

	ResizeGBuffer(albedo  , desc.Width, desc.Height);
	ResizeGBuffer(world   , desc.Width, desc.Height);
	ResizeGBuffer(normal  , desc.Width, desc.Height);
	ResizeGBuffer(specular, desc.Width, desc.Height);
}

void Update() {
	static bool fon = false;
	if (input::GetKeyState(SPACE) == input::key_state::JUST_PRESSED) {
		fon = !fon;
		input::SetCursorVisible(!input::IsCursorVisible());
		input::SetCursorLock(!input::IsCursorLocked());
	}

	if (fon) {
		UpdateFlycam(MAIN_CAMERA.transform, 10.0f, 0.1f);
	}
}

void FixedUpdate() {

}

void Render() {

	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.0f };
	pn::ClearRenderTargetView(albedo.render_target, color);
	pn::ClearRenderTargetView(normal.render_target, color);
	pn::ClearRenderTargetView(world.render_target, color);
	pn::ClearRenderTargetView(specular.render_target, color);
	
	ImGui::Begin("Lights");
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		ImGui::PushID(i);
		gui::EditStruct(lights[i]);
		ImGui::PopID();
	}
	ImGui::End();
	
	ImGui::Begin("Environment");
	gui::EditStruct(environment_lighting.data);
	ImGui::End();

	UpdateBuffer(environment_lighting);

	ImGui::Begin("Material");
	gui::EditStruct(material.data);
	ImGui::End();

	UpdateBuffer(material);

	ImGui::Begin("Camera");
	gui::EditStruct(MAIN_CAMERA.transform);
	ImGui::End();

	// ------ BEGIN RENDER

	SetDepthTest(true);
	
	ID3D11RenderTargetView * const gbuffers[4] = {
		albedo.render_target.Get(),
		world.render_target.Get(),
		normal.render_target.Get(),
		specular.render_target.Get()
	};

	_context->OMSetRenderTargets(4, gbuffers, DISPLAY_DEPTH_STENCIL.Get());

	SetStandardShaderProgram(gbuffer_fill_shader);

	SetProgramResource("albedo", dragon_albedo);
	SetProgramResource("roughness", dragon_rough);

	SetVertexBuffers(sphere_face_mesh);
	gui::EditStruct(sphere_face_transform);
	UpdateModelConstantCBuffer(sphere_face_transform);
	DrawIndexed(sphere_face_mesh);

	/*SetVertexBuffers(sphere_body_mesh);
	gui::EditStruct(sphere_body_transform);
	UpdateModelConstantCBuffer(sphere_body_transform);
	DrawIndexed(sphere_body_mesh);
	*/
	SetRenderTarget(DISPLAY_RENDER_TARGET, nullptr);
	SetDepthTest(false);
	SetBlendState(additive_blend);

	// --- ENVIRONMENT LIGHTING ---

	SetStandardShaderProgram(deferred_env_lighting_shader);
	SetVertexBuffersScreen();
	
	SetProgramSampler("ss", ss);
	SetProgramResource("albedo", albedo.texture);
	SetProgramResource("world", world.texture);
	SetProgramResource("normal", normal.texture);
	SetProgramResource("specular", specular.texture);
	
	SetProgramConstant("material", material);
	SetProgramConstant("environment_lighting", environment_lighting);

	SetProgramResource("environment", cubemap);

	_context->Draw(4, 0);

	// --- DIRECT LIGHTING ---

	SetStandardShaderProgram(deferred_lighting_shader);
	SetVertexBuffersScreen();	
	
	SetProgramSampler("ss", ss);
	SetProgramResource("albedo", albedo.texture);
	SetProgramResource("world", world.texture);
	SetProgramResource("normal", normal.texture);
	SetProgramResource("specular", specular.texture);
	SetProgramConstant("light", light);
	
	SetProgramConstant("material", material);

	for (int i = 0; i < NUM_LIGHTS; ++i) {
		light.data = lights[i];
		UpdateBuffer(light);
		_context->Draw(4, 0);
	}

	SetBlendState();

	// ---- RENDER ENVIRONMENT ---

	SetStandardShaderProgram(render_cubemap);
	SetRenderTarget(DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);
	SetDepthTest(true);

	CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	auto depth_stencil_state = CreateDepthStencilState(&desc);
	SetDepthStencilState(depth_stencil_state);

	SetVertexBuffers(cubemap_mesh_buffer);
	cubemap_transform.position = MAIN_CAMERA.transform.position;
	UpdateModelConstantCBuffer(cubemap_transform);
	SetProgramResource("cubemap", cubemap);
	DrawIndexed(cubemap_mesh_buffer);

	/*SetShaderProgram(simple_texture_shader);
	
	SetDepthTest(false);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	SetProgramSampler("ss", ss);

	auto hw = app::window_desc.width / 2;
	auto hh = app::window_desc.height / 2;

	SetViewport(hw, hh, 0, 0);
	SetProgramResource("display_texture", normal.texture);
	_context->Draw(4,0);
	
	SetViewport(hw, hh, hw, 0);
	SetProgramResource("display_texture", albedo.texture);
	_context->Draw(4, 0);

	SetViewport(hw, hh, 0, hh);
	SetProgramResource("display_texture", world.texture);
	_context->Draw(4, 0);

	SetViewport(hw, hh, hw, hh);
	SetProgramResource("display_texture", specular.texture);
	_context->Draw(4, 0);
	
	SetViewport(app::window_desc.width, app::window_desc.height);*/
}

void MainLoopBegin() {

}

void MainLoopEnd() {

}

void Close() {

}