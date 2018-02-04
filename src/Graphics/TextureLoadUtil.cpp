#include <Graphics\TextureLoadUtil.h>

#include <IO\FileUtil.h>

#include <memory>

#include <DirectXTK\WICTextureLoader.h>
#include <DirectXTK\DDSTextureLoader.h>

namespace pn {

// ------ FUNCTIONS ---------


dx_resource_view LoadTexture2D(const string& filepath) {
	auto image_data = ReadResource(filepath);
	
	dx_resource_view resource_view;
	auto hr = DirectX::CreateWICTextureFromMemoryEx(
		_device.Get(), _context.Get(),
		(const uint8_t*)(image_data.data()), image_data.size(), 0, 
		D3D11_USAGE::D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
		DirectX::WIC_LOADER_IGNORE_SRGB,
		nullptr, resource_view.GetAddressOf()
	);
	
	if (FAILED(hr)) {
		LogError("Couldn't create texture: {}", ErrMsg(hr));
	}

	return resource_view;
}

dx_resource_view LoadCubemap(const string& filepath) {
	auto image_data = ReadResource(filepath);

	dx_resource_view cubemap_view;
	auto hr = DirectX::CreateDDSTextureFromMemoryEx(
		_device.Get(), _context.Get(),
		(const uint8_t*) (image_data.data()), image_data.size(), 0,
		D3D11_USAGE::D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0, 0, false,
		nullptr, cubemap_view.GetAddressOf(), 
		nullptr
		);

	if (FAILED(hr)) {
		LogError("Couldn't create texture: {}", ErrMsg(hr));
	}

	return cubemap_view;
}

} // namespace pn