#include <Graphics\GBuffer.h>

namespace pn {

void InitGBuffer(gbuffer_t& g, CD3D11_TEXTURE2D_DESC d) {
	auto tex = pn::CreateTexture2D(d);
	g.texture = pn::CreateShaderResourceView(tex);
	g.render_target = pn::CreateRenderTargetView(tex);
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

}