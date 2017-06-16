#pragma once

#include <d3d11_1.h>
#include <dxgi1_3.h>
#include <d3d11shader.h>

#include <wrl.h> // ComPtr

#include <Graphics\Window.h>
#include <Utilities\UtilityTypes.h>
#include <Utilities\Logging.h>

namespace pn {

// ------------ TYPEDEFS -------------

template<typename T>
using dx_ptr				= Microsoft::WRL::ComPtr<T>;

using vertex_input_desc		= pn::vector<struct input_element_desc>;

using dx_device				= dx_ptr<ID3D11Device>;
using dx_context			= dx_ptr<ID3D11DeviceContext>;
using dx_swap_chain			= dx_ptr<IDXGISwapChain>;
using dx_buffer				= dx_ptr<ID3D11Buffer>;

using dx_render_target_view = dx_ptr<ID3D11RenderTargetView>;
using dx_depth_stencil_view = dx_ptr<ID3D11DepthStencilView>;

using dx_input_layout		= dx_ptr<ID3D11InputLayout>;
using dx_shader_reflection	= dx_ptr<ID3D11ShaderReflection>;
using dx_vertex_shader		= dx_ptr<ID3D11VertexShader>;
using dx_pixel_shader		= dx_ptr<ID3D11PixelShader>;

using dx_texture2d			= dx_ptr<ID3D11Texture2D>;

// ------------- CLASS DEFINITIONS ---------------

struct input_element_desc {
	LPCSTR                     SemanticName;
	UINT                       SemanticIndex;
	DXGI_FORMAT                Format;
	UINT                       InputSlot;
	UINT                       AlignedByteOffset;
	D3D11_INPUT_CLASSIFICATION InputSlotClass;
	UINT                       InstanceDataStepRate;
	static const int NAME_SIZE = 16;
	
	input_element_desc(const D3D11_INPUT_ELEMENT_DESC& d3d_desc) {
		SemanticName = new char[NAME_SIZE];
		strcpy(const_cast<char*>(SemanticName), d3d_desc.SemanticName);

		SemanticIndex		 = d3d_desc.SemanticIndex;
		Format				 = d3d_desc.Format;
		InputSlot			 = d3d_desc.InputSlot;
		AlignedByteOffset	 = d3d_desc.AlignedByteOffset;
		InputSlotClass		 = d3d_desc.InputSlotClass;
		InstanceDataStepRate = d3d_desc.InstanceDataStepRate;
	}

	~input_element_desc() {
		delete[] SemanticName;
	}
};

struct input_layout_desc {
	input_layout_desc(dx_input_layout ptr, vertex_input_desc desc) :
		ptr{ ptr }, desc{ desc } {}

	dx_input_layout ptr;
	vertex_input_desc desc;
};

struct MeshBuffer {
	dx_buffer positions;
	dx_buffer colors;
	dx_buffer normals;
	dx_buffer tangents;
	dx_buffer bitangents;
	dx_buffer uvs;
	dx_buffer uv2s;

	dx_buffer indices;
	D3D_PRIMITIVE_TOPOLOGY topology;
};

// ------------------- FUNCTIONS ----------------------

// ------------ CREATION FUNCTIONS -------------

dx_device				CreateDevice();
dx_swap_chain			CreateSwapChain(dx_device device, DXGI_SWAP_CHAIN_DESC swap_chain_desc);
dx_swap_chain			CreateMainWindowSwapChain(dx_device device, const window_handle hwnd, const application_window_desc awd);

dx_texture2d			CreateTexture2D(dx_device device, CD3D11_TEXTURE2D_DESC texture_desc, const D3D11_SUBRESOURCE_DATA* initial_data);
dx_render_target_view	CreateRenderTargetViewFromTexture(dx_device device, dx_texture2d texture);
dx_depth_stencil_view	CreateDepthStencilView(dx_device device, dx_texture2d& depth_stencil_texture);

// -------------- SHADER CREATION -------------

template<typename ID3D11ShaderType, typename DeviceShaderFunc>
auto CreateShader(dx_device device, const pn::bytes& bytes, DeviceShaderFunc CreateShader) {

	dx_ptr<ID3D11ShaderType> shader;
	if (bytes.empty()) {
		LogError("Couldn't create shader {} from byte code: {}", TYPENAME(ID3D11ShaderType), "No data");
		return shader;
	}

	auto hr = CreateShader(
		bytes.data(),
		bytes.size(),
		nullptr,
		&shader
	);

	if (FAILED(hr)) {
		LogError("Couldn't create shader {} from byte code: {}", TYPENAME(ID3D11ShaderType), ErrMsg(hr));
	}

	return shader;
}

dx_vertex_shader CreateVertexShader(dx_device device, const pn::bytes& bytes);
dx_vertex_shader CreateVertexShader(dx_device device, const std::string& filename);

dx_pixel_shader CreatePixelShader(dx_device device, const pn::bytes& ps_data);
dx_pixel_shader CreatePixelShader(dx_device device, const std::string& filename);

input_layout_desc CreateInputLayout(dx_device device, const pn::bytes& bytes, const vertex_input_desc& desc);
input_layout_desc CreateInputLayout(dx_device device, const pn::bytes& bytes);

std::pair<dx_vertex_shader, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const std::string& filename, const vertex_input_desc& desc);
std::pair<dx_vertex_shader, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const pn::bytes& vs_byte_code, const vertex_input_desc& desc);
std::pair<dx_vertex_shader, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const std::string& filename);

// Update the render target and depth stencil views to the size of the back buffer
void SetRenderTargetViewAndDepthStencilFromSwapChain(dx_device device, dx_swap_chain swap_chain, dx_render_target_view& render_target_view, dx_depth_stencil_view& depth_stencil_view);

// -------------- BUFFER CREATION -------------------

template<typename BufferDataType>
auto CreateBufferDataDesc(BufferDataType* data) {
	D3D11_SUBRESOURCE_DATA data_desc;
	ZeroMemory(&data_desc, sizeof(decltype(data_desc)));
	data_desc.pSysMem = data;
	data_desc.SysMemPitch = 0;
	data_desc.SysMemSlicePitch = 0;
	return data_desc;
}

template<typename BufferDataType>
auto CreateBuffer(dx_device device, BufferDataType* v_data, const unsigned int n, const D3D11_BIND_FLAG bind_target) {
	// Create buffer description
	CD3D11_BUFFER_DESC buffer_desc(n * sizeof(BufferDataType), bind_target);

	// Create buffer data
	auto data_desc = CreateBufferDataDesc(v_data);

	dx_buffer buffer;
	auto hr = device->CreateBuffer(&buffer_desc, &data_desc, &buffer);
	if (FAILED(hr)) {
		LogError("Couldn't create buffer {}: {}", bind_target, pn::ErrMsg(hr));
	}

	return buffer;
}

template<typename VertexDataType>
auto CreateVertexBuffer(dx_device device, const VertexDataType* v_data, const unsigned int n) {
	return CreateBuffer(device, v_data, n, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER);
}

template<typename VertexDataType>
auto CreateVertexBuffer(dx_device device, const pn::vector<VertexDataType>& v_data) {
	return CreateVertexBuffer(device, v_data.data(), v_data.size());
}

template<typename ConstantDataType>
auto CreateConstantBuffer(dx_device device, const ConstantDataType* c_data, const unsigned int n) {
	static_assert((sizeof(ConstantDataType) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	return CreateBuffer(device, c_data, n, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER);
}

template<typename ConstantDataType>
auto CreateConstantBuffer(dx_device device, const pn::vector<ConstantDataType>& c_data) {
	return CreateConstantBuffer(device, c_data.data(), c_data.size());
}

template<typename IndexDataType>
auto CreateIndexBuffer(dx_device device, const IndexDataType* i_data, const unsigned int n) {
	return CreateBuffer(device, i_data, n, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER);
}

template<typename IndexDataType>
auto CreateIndexBuffer(dx_device device, const pn::vector<IndexDataType>& i_data) {
	return CreateIndexBuffer(device, i_data.data(), i_data.size());
}

// ------------ UTILITY FUNCTIONS -------------

dx_texture2d GetSwapChainBackBuffer(dx_swap_chain swap_chain);
dx_context GetContext(dx_device device);
CD3D11_TEXTURE2D_DESC GetTextureDesc(dx_texture2d texture);

dx_shader_reflection GetShaderReflector(const pn::bytes& shader_byte_code);
vertex_input_desc GetVertexInputDescFromShader(const pn::bytes& vs_byte_code);

void SetViewport(dx_context context, const int width, const int height);

} // namespace pn