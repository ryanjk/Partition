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
	DragFloat("intensity", &light.light_intensity, 0, 100);
}

#define NUM_LIGHTS 1
light_t lights[NUM_LIGHTS];
cbuffer<light_t> light;

// ---------------------------------

struct alignas(16) material_t {
	float roughness;
	float reflectivity;
};

template<>
void gui::EditStruct(material_t& material) {
	DragFloat("roughness", &material.roughness, 0, 10);
	DragFloat("reflectivity", &material.reflectivity, 0, 10);
}

cbuffer<material_t> material;

// --- gbuffer data ---

struct gbuffer {
	pn::dx_render_target_view render_target;
	pn::dx_resource_view      texture;
};

gbuffer albedo;
gbuffer world;
gbuffer normal;
gbuffer specular;

pn::shader_program_t gbuffer_fill_shader;
pn::shader_program_t simple_texture_shader;
pn::shader_program_t deferred_lighting_shader;

rdb::mesh_resource_t scene_mesh;

transform_t dragon_transform;

pn::dx_sampler_state	ss;

dx_blend_state additive_blend;

void Init() {

	// ---------- LOAD RESOURCES ----------------

	pn::StartProfile("Loading all meshes");

	{
		pn::StartProfile("Loading dragon mesh");
		auto mesh_handle = pn::LoadMesh(pn::GetResourcePath("dragon.fbx"));
		pn::EndProfile();

		scene_mesh = pn::rdb::GetMeshResource("default");
	}

	pn::EndProfile();

	dragon_transform.position = vec3f(0.0f, -4.0f, 9.2f);

	// --------- CREATE SHADER DATA ---------------

	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	ss = pn::CreateSamplerState(sampler_desc);

	gbuffer_fill_shader      = pn::CompileShaderProgram(pn::GetResourcePath("gbuffer_fill.hlsl"));
	simple_texture_shader    = pn::CompileShaderProgram(pn::GetResourcePath("simple_texture.hlsl"));
	deferred_lighting_shader = pn::CompileShaderProgram(pn::GetResourcePath("deferred_lighting.hlsl"));

	// ---- CREATE GBUFFERS -----

	auto back_buffer = pn::GetSwapChainBuffer(SWAP_CHAIN);
	auto back_buffer_desc = pn::GetDesc(back_buffer);
	back_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	auto InitGBuffer = [](gbuffer& g, CD3D11_TEXTURE2D_DESC d) {
		auto tex        = pn::CreateTexture2D(d);
		g.texture       = pn::CreateShaderResourceView(tex);
		g.render_target = pn::CreateRenderTargetView(tex);
	};
	
	back_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	InitGBuffer(albedo  , back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R32_FLOAT;
	InitGBuffer(world   , back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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

	// ----- INIT MATERIAL ------

	InitializeCBuffer(material);
	material.data.roughness    = 0.5f;
	material.data.reflectivity = 0.5f;

	// ----- CREATE BLEND DESC -------

	CD3D11_BLEND_DESC blend_desc(D3D11_DEFAULT);
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additive_blend = CreateBlendState(&blend_desc);
}

void Resize() {
	auto ResizeGBuffer = [](gbuffer& g, CD3D11_TEXTURE2D_DESC d) {		
		D3D11_RENDER_TARGET_VIEW_DESC rt_desc;
		g.render_target->GetDesc(&rt_desc);
		d.Format = rt_desc.Format;

		g.render_target.ReleaseAndGetAddressOf();
		g.texture.ReleaseAndGetAddressOf();
		
		auto tex = pn::CreateTexture2D(d);
		g.texture       = pn::CreateShaderResourceView(tex);
		g.render_target = pn::CreateRenderTargetView(tex);
	};

	CD3D11_TEXTURE2D_DESC desc = GetDesc(GetSwapChainBuffer(SWAP_CHAIN));
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	ResizeGBuffer(albedo  , desc);
	ResizeGBuffer(world   , desc);
	ResizeGBuffer(normal  , desc);
	ResizeGBuffer(specular, desc);
}

void Update() {}

void FixedUpdate() {}

void Render() {

	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.000f };
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
	
	ImGui::Begin("Material");
	gui::EditStruct(material.data);
	ImGui::End();

	UpdateBuffer(material);

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
	SetVertexBuffers(scene_mesh);

	gui::EditStruct(dragon_transform);
	UpdateModelConstantCBuffer(dragon_transform);

	DrawIndexed(scene_mesh);

	// --- DISPLAY GBUFFERS ---

	SetRenderTarget(DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);

	SetStandardShaderProgram(deferred_lighting_shader);
	SetDepthTest(false);
	SetBlendState(additive_blend);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
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