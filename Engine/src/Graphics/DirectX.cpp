#include <Graphics\DirectX.h>

#include <functional>
#include <memory>

#include <d3dcompiler.h>

#include <IO\FileUtil.h>

using namespace std::placeholders;

namespace pn {

// --------------- CONSTANTS ---------------
static const D3D_FEATURE_LEVEL SUPPORTED_D3D_FEATURE_LEVELS[] = {
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

static const D3D_FEATURE_LEVEL BAD_LEVELS[] = {
	D3D_FEATURE_LEVEL_12_0
};

// --------------- FUNCTIONS --------------------

// ------------ CREATION FUNCTIONS -------------

dx_device CreateDevice() {

	auto GetDeviceFlags = []() {
		UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		return deviceFlags;
	};

	auto device_flags = GetDeviceFlags();
	D3D_FEATURE_LEVEL feature_level;
	dx_device device;
	dx_context context;
	auto hr = D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		device_flags,                // Set debug and Direct2D compatibility flags.
		SUPPORTED_D3D_FEATURE_LEVELS,                     // List of feature levels this app can support.
		ARRAYSIZE(SUPPORTED_D3D_FEATURE_LEVELS),          // Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,                    // Returns the Direct3D device created.
		&feature_level,            // Returns feature level of device created.
		&context                    // Returns the device immediate context.
	);

	if (FAILED(hr)) {
		// Handle device interface creation failure if it occurs.
		// For example, reduce the feature level requirement, or fail over 
		// to WARP rendering.
		LogError("Couldn't create D3D11 device: {}", ErrMsg(hr));
		exit(1);
	}

	Log("Loaded DirectX Version {0:x}", feature_level);

	dx_ptr<IDXGIDevice> gi_device;
	dx_ptr<IDXGIAdapter> adapter;
	device->QueryInterface(__uuidof(IDXGIDevice), (void**) gi_device.GetAddressOf());
	gi_device->GetAdapter(adapter.GetAddressOf());
	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);
	char description[256];
	wcstombs(description, desc.Description, 256);
	Log("Graphics Card: {}", description);
	return device;

}

dx_swap_chain CreateSwapChain(dx_device device, DXGI_SWAP_CHAIN_DESC swap_chain_desc) {
	// Create the DXGI device object to use in other factories, such as Direct2D.
	dx_ptr<IDXGIDevice3> dxgiDevice;
	device.As(&dxgiDevice);

	// Create swap chain.
	dx_ptr<IDXGIAdapter> adapter;
	dx_ptr<IDXGIFactory> factory;

	auto hr = dxgiDevice->GetAdapter(&adapter);
	dx_swap_chain swap_chain;
	if (SUCCEEDED(hr)) {
		adapter->GetParent(IID_PPV_ARGS(&factory));
		hr = factory->CreateSwapChain(
			device.Get(),
			&swap_chain_desc,
			&swap_chain
		);
		if (FAILED(hr)) {
			LogError("Couldn't create swap chain: {}", ErrMsg(hr));
			exit(1);
		}
	}
	else {
		LogError("Couldn't get device adapter: {}", ErrMsg(hr));
		exit(1);
	}
	return swap_chain;
}

dx_swap_chain CreateMainWindowSwapChain(dx_device device, const HWND hwnd, const application_window_desc awd) {
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));
	desc.Windowed = !awd.fullscreen;
	desc.BufferCount = 2;
	desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.BufferDesc.Width = awd.width;
	desc.BufferDesc.Height = awd.height;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SampleDesc.Count = 1;      //multisampling setting
	desc.SampleDesc.Quality = 0;    //vendor-specific flag
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.OutputWindow = hwnd;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

	return CreateSwapChain(device, desc);
}

dx_ptr<ID3D11RenderTargetView> CreateRenderTargetViewOfTexture(dx_device device, dx_ptr<ID3D11Texture2D> texture) {
	dx_ptr<ID3D11RenderTargetView> render_target_view;
	auto hr = device->CreateRenderTargetView(
		texture.Get(),
		nullptr,
		render_target_view.GetAddressOf()
	);

	if (FAILED(hr)) {
		LogError("Couldn't create render target view: {}", ErrMsg(hr));
	}

	return render_target_view;
}

dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(dx_device device, dx_ptr<ID3D11Texture2D>& depth_stencil_texture) {
	dx_ptr<ID3D11DepthStencilView> depth_stencil_view;
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	auto hr = device->CreateDepthStencilView(
		depth_stencil_texture.Get(),
		&depthStencilViewDesc,
		&depth_stencil_view
	);

	if (FAILED(hr)) {
		LogError("Couldn't create depth stencil view: {}", ErrMsg(hr));
	}

	return depth_stencil_view;
}

dx_ptr<ID3D11Texture2D> CreateTexture2D(dx_device device, CD3D11_TEXTURE2D_DESC texture_desc, const D3D11_SUBRESOURCE_DATA* initial_data = nullptr) {
	dx_ptr<ID3D11Texture2D> texture;
	auto hr = device->CreateTexture2D(
		&texture_desc,
		initial_data,
		&texture
	);

	if (FAILED(hr)) {
		LogError("Couldn't create texture 2D: {}", ErrMsg(hr));
	}

	return texture;
}

// -------------- SHADER CREATION -------------

dx_ptr<ID3D11VertexShader> CreateVertexShader(dx_ptr <ID3D11Device> device, const pn::bytes& bytes) {
	return CreateShader<ID3D11VertexShader>(device, bytes, std::bind(&ID3D11Device::CreateVertexShader, device.Get(), _1, _2, _3, _4));
}

dx_ptr<ID3D11VertexShader> CreateVertexShader(dx_ptr <ID3D11Device> device, const std::string& filename) {
	auto vs_data = pn::ReadFile(filename);
	return CreateVertexShader(device, vs_data);
}

dx_ptr<ID3D11PixelShader> CreatePixelShader(dx_ptr <ID3D11Device> device, const pn::bytes& ps_data) {
	return CreateShader<ID3D11PixelShader>(device, ps_data, std::bind(&ID3D11Device::CreatePixelShader, device.Get(), _1, _2, _3, _4));
}

dx_ptr<ID3D11PixelShader> CreatePixelShader(dx_ptr <ID3D11Device> device, const std::string& filename) {
	auto ps_data = pn::ReadFile(filename);
	return CreatePixelShader(device, ps_data);
}


input_layout_desc CreateInputLayout(dx_ptr <ID3D11Device> device, const pn::bytes& vs_byte_code, const vertex_input_desc& desc) {

	dx_ptr<ID3D11InputLayout> ptr;
	auto hr = device->CreateInputLayout(
		reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(desc.data()),
		desc.size(),
		vs_byte_code.data(),
		vs_byte_code.size(),
		&ptr
	);

	if (FAILED(hr)) {
		LogError("Couldn't create input layout: {}", ErrMsg(hr));
	}

	auto layout = input_layout_desc(ptr, desc);
	return layout;
}

input_layout_desc CreateInputLayout(dx_ptr <ID3D11Device> device, const pn::bytes& vs_byte_code) {
	auto desc = GetVertexInputDescFromShader(vs_byte_code);
	return CreateInputLayout(device, vs_byte_code, desc);
}

std::pair<dx_ptr<ID3D11VertexShader>, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const pn::bytes& vs_data, const vertex_input_desc& desc) {
	auto vertex_shader = CreateVertexShader(device, vs_data);
	auto input_layout = CreateInputLayout(device, vs_data, desc);
	return std::make_pair(vertex_shader, input_layout);
}

std::pair<dx_ptr<ID3D11VertexShader>, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const std::string& filename, const vertex_input_desc& desc) {
	auto vs_data = pn::ReadFile(filename);
	return CreateVertexShaderAndInputLayout(device, vs_data, desc);
}

std::pair<dx_ptr<ID3D11VertexShader>, input_layout_desc> CreateVertexShaderAndInputLayout(dx_device device, const std::string& filename) {
	auto vs_data = pn::ReadFile(filename);
	vertex_input_desc desc = GetVertexInputDescFromShader(vs_data);
	return CreateVertexShaderAndInputLayout(device, vs_data, desc);
}

CD3D11_TEXTURE2D_DESC GetTextureDesc(dx_ptr<ID3D11Texture2D> texture) {
	CD3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);
	return desc;
}

void SetRenderTargetViewAndDepthStencilFromSwapChain(
	dx_device device,
	dx_swap_chain swap_chain,
	dx_ptr<ID3D11RenderTargetView>& render_target_view,
	dx_ptr<ID3D11DepthStencilView>& depth_stencil_view) {
	auto back_buffer = pn::GetSwapChainBackBuffer(swap_chain);
	render_target_view = pn::CreateRenderTargetViewOfTexture(device, back_buffer);

	CD3D11_TEXTURE2D_DESC back_buffer_desc = pn::GetTextureDesc(back_buffer);

	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT> (back_buffer_desc.Width),
		static_cast<UINT> (back_buffer_desc.Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);

	auto depth_stencil = pn::CreateTexture2D(device, depthStencilDesc);
	depth_stencil_view = pn::CreateDepthStencilView(device, depth_stencil);

	auto context = pn::GetContext(device);
	context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());
}

// ------------ UTILITY FUNCTIONS -------------

dx_ptr<ID3D11Texture2D> GetSwapChainBackBuffer(dx_swap_chain swap_chain) {
	dx_ptr<ID3D11Texture2D> back_buffer;
	auto hr = swap_chain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**) &back_buffer);

	if (FAILED(hr)) {
		// TODO: Fail condition
		LogError("Couldn't get swap chain buffer: {}", ErrMsg(hr));
	}

	return back_buffer;
}

dx_context GetContext(dx_device device) {
	dx_context context;
	device->GetImmediateContext(&context);

	if (!context) {
		LogError("Couldn't get device context");
	}

	return context;
}

void SetViewport(dx_context context, const int width, const int height) {
	D3D11_VIEWPORT m_viewport;
	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.Height = (float) height;
	m_viewport.Width = (float) width;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1;

	context->RSSetViewports(
		1,
		&m_viewport
	);
}

vertex_input_desc GetVertexInputDescFromShader(const pn::bytes& vs_byte_code) {
	dx_ptr<ID3D11ShaderReflection> reflector = GetShaderReflector(vs_byte_code);
	D3D11_SHADER_DESC shader_desc;
	auto hr = reflector->GetDesc(&shader_desc);
	if (FAILED(hr)) {
		LogError("Couldn't get vertex shader description from reflector");
		return {};
	}

	vertex_input_desc vertex_desc;
	vertex_desc.reserve(shader_desc.InputParameters);
	for (unsigned int i = 0; i < shader_desc.InputParameters; ++i) {
		D3D11_SIGNATURE_PARAMETER_DESC param_desc;
		auto hr = reflector->GetInputParameterDesc(i, &param_desc);
		if (FAILED(hr)) {
			LogError("Couldn't get input parameter description from reflector");
		}
		D3D11_INPUT_ELEMENT_DESC element_desc{};
		//char* name = const_cast<LPSTR>(element_desc.SemanticName);
		LogDebug("Creating element description");
		element_desc.SemanticName = new char[16];
		strcpy(const_cast<LPSTR>(element_desc.SemanticName), param_desc.SemanticName);
		element_desc.SemanticIndex = param_desc.SemanticIndex;
		element_desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element_desc.InputSlot = i;
		element_desc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element_desc.InstanceDataStepRate = 0;

		if (param_desc.Mask == 1) {
			if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) element_desc.Format = DXGI_FORMAT_R32_UINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) element_desc.Format = DXGI_FORMAT_R32_SINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) element_desc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (param_desc.Mask <= 3) {
			if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) element_desc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) element_desc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) element_desc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (param_desc.Mask <= 7) {
			if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) element_desc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) element_desc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) element_desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (param_desc.Mask <= 15) {
			if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) element_desc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) element_desc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) element_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		vertex_desc.push_back(element_desc);
	}

	return vertex_desc;
}

dx_ptr<ID3D11ShaderReflection> GetShaderReflector(const pn::bytes& shader_byte_code) {
	dx_ptr<ID3D11ShaderReflection> reflector = nullptr;
	auto hr = D3DReflect(shader_byte_code.data(), shader_byte_code.size(), IID_ID3D11ShaderReflection, (void**) reflector.GetAddressOf());
	if (FAILED(hr)) {
		LogError("Couldn't get reflector from shader");
		return nullptr;
	}
	return reflector;
}

} // namespace dx