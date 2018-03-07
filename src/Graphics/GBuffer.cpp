#include <Graphics\GBuffer.h>
#include <Graphics\RenderSystem.h>

#include <IO\PathUtil.h>

namespace pn {

// -------------
// --- STATE ---
// -------------

gbuffer_t ALBEDO_GBUFFER;
gbuffer_t DEPTH_GBUFFER;
gbuffer_t NORMAL_GBUFFER;
gbuffer_t SPECULAR_GBUFFER;

shader_program_t GBUFFER_FILL;
shader_program_t DEFERRED_LIGHTING;
shader_program_t DEFERRED_CUBEMAP_LIGHTING;

// -----------------
// --- FUNCTIONS ---
// -----------------

template<>
void gui::EditStruct(deferred_material_t& material) {
	DragFloat3("albedo", &material.albedoMetal.x, 0, 1);
	DragFloat3("specular", &material.specRoughness.x, 0, 1);
	DragFloat("roughness", &material.specRoughness.w, 0, 1, 0.2f);
	DragFloat("metallic", &material.albedoMetal.w, 0, 1, 0.2f);
}

void InitGBuffers() {

	// --- INIT TEXTURES ---

	auto back_buffer           = pn::GetSwapChainBuffer(SWAP_CHAIN);
	auto back_buffer_desc      = pn::GetDesc(back_buffer);
	back_buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	back_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	InitGBuffer(ALBEDO_GBUFFER, back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R32_FLOAT;
	InitGBuffer(DEPTH_GBUFFER, back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
	InitGBuffer(NORMAL_GBUFFER, back_buffer_desc);

	back_buffer_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	InitGBuffer(SPECULAR_GBUFFER, back_buffer_desc);

	// --- INIT SHADERS ---

	GBUFFER_FILL = pn::CompileShaderProgram(pn::GetResourcePath("gbuffer_fill.hlsl"));
	DEFERRED_LIGHTING = pn::CompileShaderProgram(pn::GetResourcePath("deferred_lighting.hlsl"));
	DEFERRED_CUBEMAP_LIGHTING = pn::CompileShaderProgram(pn::GetResourcePath("deferred_env_lighting.hlsl"));
}

void InitGBuffer(gbuffer_t& g, CD3D11_TEXTURE2D_DESC d) {
	auto tex        = pn::CreateTexture2D(d);
	g.texture       = pn::CreateShaderResourceView(tex);
	g.render_target = pn::CreateRenderTargetView(tex);
}

void ResizeGBuffers() {
	CD3D11_TEXTURE2D_DESC desc = GetDesc(GetSwapChainBuffer(SWAP_CHAIN));

	ResizeGBuffer(ALBEDO_GBUFFER  , desc.Width, desc.Height);
	ResizeGBuffer(DEPTH_GBUFFER   , desc.Width, desc.Height);
	ResizeGBuffer(NORMAL_GBUFFER  , desc.Width, desc.Height);
	ResizeGBuffer(SPECULAR_GBUFFER, desc.Width, desc.Height);
}

void ResizeGBuffer(gbuffer_t& g, int width, int height) {
	dx_resource resource;
	g.texture->GetResource(resource.GetAddressOf());

	dx_texture2d texture_resource;
	resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(texture_resource.GetAddressOf()));

	CD3D11_TEXTURE2D_DESC texture_desc;
	texture_resource->GetDesc(&texture_desc);

	texture_desc.Width = width;
	texture_desc.Height = height;

	g.render_target.ReleaseAndGetAddressOf();
	g.texture.ReleaseAndGetAddressOf();

	InitGBuffer(g, texture_desc);
}

void ClearGBufferRenderTargets() {
	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.0f };
	pn::ClearRenderTargetView(ALBEDO_GBUFFER.render_target  , color);
	pn::ClearRenderTargetView(NORMAL_GBUFFER.render_target  , color);
	pn::ClearRenderTargetView(DEPTH_GBUFFER.render_target   , color);
	pn::ClearRenderTargetView(SPECULAR_GBUFFER.render_target, color);
}

void SetGBufferRenderTargets() {
	ID3D11RenderTargetView * const gbuffers[4] = {
		ALBEDO_GBUFFER.render_target.Get(),
		DEPTH_GBUFFER.render_target.Get(),
		NORMAL_GBUFFER.render_target.Get(),
		SPECULAR_GBUFFER.render_target.Get()
	};

	_context->OMSetRenderTargets(4, gbuffers, DISPLAY_DEPTH_STENCIL.Get());
}

void SetDeferredShaderProgram(shader_program_t& shader) {
	SetStandardShaderProgram(shader);
	SetVertexBuffersScreen();
	SetProgramResource("albedo"  , ALBEDO_GBUFFER.texture);
	SetProgramResource("world"   , DEPTH_GBUFFER.texture);
	SetProgramResource("normal"  , NORMAL_GBUFFER.texture);
	SetProgramResource("specular", SPECULAR_GBUFFER.texture);
}

}