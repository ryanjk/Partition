#pragma once

#include <Graphics\DirectX.h>

namespace pn {

struct gbuffer_t {
	pn::dx_render_target_view render_target;
	pn::dx_resource_view      texture;
};

void InitGBuffer(gbuffer_t& g, CD3D11_TEXTURE2D_DESC d);

void ResizeGBuffer(gbuffer_t& g, int width, int height);

}


