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

// --- gbuffer data ---

struct gbuffer {
	pn::dx_render_target_view render_target;
	pn::dx_resource_view      texture;
};

gbuffer albedo;
gbuffer world;
gbuffer normal;
gbuffer specular;

pn::shader_program_t      gbuffer_fill_shader;
pn::shader_program_t      simple_texture_shader;

rdb::mesh_resource_t scene_mesh;

transform_t dragon_transform;

pn::dx_sampler_state	ss;

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

	dragon_transform.position = vec3f(0, -4, 9.2);

	// --------- CREATE SHADER DATA ---------------

	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	ss = pn::CreateSamplerState(sampler_desc);

	gbuffer_fill_shader   = pn::CompileShaderProgram(pn::GetResourcePath("gbuffer_fill.hlsl"));
	simple_texture_shader = pn::CompileShaderProgram(pn::GetResourcePath("simple_texture.hlsl"));

	// ---- CREATE GBUFFERS -----

	auto back_buffer = pn::GetSwapChainBuffer(SWAP_CHAIN);
	auto back_buffer_desc = pn::GetDesc(back_buffer);
	back_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	auto InitGBuffer = [](gbuffer& g, CD3D11_TEXTURE2D_DESC d) {
		auto tex        = pn::CreateTexture2D(d);
		g.texture       = pn::CreateShaderResourceView(tex);
		g.render_target = pn::CreateRenderTargetView(tex);
	};
	
	InitGBuffer(albedo  , back_buffer_desc);
	InitGBuffer(world   , back_buffer_desc);
	InitGBuffer(normal  , back_buffer_desc);
	InitGBuffer(specular, back_buffer_desc);

}

void Update(const double dt) {}

void Render() {

	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.000f };
	pn::ClearRenderTargetView(albedo.render_target, color);
	pn::ClearRenderTargetView(normal.render_target, color);
	pn::ClearRenderTargetView(world.render_target, color);
	pn::ClearRenderTargetView(specular.render_target, color);

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

	SetShaderProgram(simple_texture_shader);
	
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
	
	SetViewport(app::window_desc.width, app::window_desc.height);
}

void MainLoopBegin() {

}

void MainLoopEnd() {

}

void Close() {

}