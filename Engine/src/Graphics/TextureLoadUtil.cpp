#include <Graphics\TextureLoadUtil.h>

#include <IO\FileUtil.h>

#include <memory>

#include <DirectXTK\WICTextureLoader.h>

namespace pn {

// ------- VARIABLES -----------

static dx_device	_device;
static dx_context	_context;

// ------ FUNCTIONS ---------

void InitTextureFactory(dx_device device) {
	_device = device;
	device->GetImmediateContext(_context.GetAddressOf());
}

texture_t LoadTexture2D(const string& filepath) {
	auto image_data = ReadFile(filepath);
	dx_resource resource;
	dx_resource_view resource_view;
	auto hr = DirectX::CreateWICTextureFromMemory(_device.Get(), _context.Get(), 
		(const uint8_t*)(image_data.data()), image_data.size(), resource.GetAddressOf(), resource_view.GetAddressOf());
	if (FAILED(hr)) {
		LogError("Couldn't create texture: {}", ErrMsg(hr));
	}

	texture_t t;
	t.resource = resource;
	t.resource_view = resource_view;
	return t;
}

} // namespace pn