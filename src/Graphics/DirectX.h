#pragma once

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>
#include <d3d12shader.h>

#include <wrl.h> // ComPtr

#include <Application\ResourceDatabaseTypes.h>

#include <Graphics\Window.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>
#include <Utilities\Math.h>
#include <Utilities\UtilityTypes.h>


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

using dx_sampler_state		= dx_ptr<ID3D11SamplerState>;
using dx_texture2d			= dx_ptr<ID3D11Texture2D>;
using dx_resource			= dx_ptr<ID3D11Resource>;
using dx_resource_view		= dx_ptr<ID3D11ShaderResourceView>;

using dx_blend_state		 = dx_ptr<ID3D11BlendState>;
using dx_depth_stencil_state = dx_ptr<ID3D11DepthStencilState>;
using dx_rasterizer_state    = dx_ptr<ID3D11RasterizerState>;

// -------- CONSTANTS --------------

extern const unsigned int DEFAULT_SHADER_COMPILATION_FLAGS;

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
	
	input_element_desc(const input_element_desc& d3d_desc) {
		SemanticName = new char[NAME_SIZE];
		strcpy(const_cast<char*>(SemanticName), d3d_desc.SemanticName);

		SemanticIndex		 = d3d_desc.SemanticIndex;
		Format				 = d3d_desc.Format;
		InputSlot			 = d3d_desc.InputSlot;
		AlignedByteOffset	 = d3d_desc.AlignedByteOffset;
		InputSlotClass		 = d3d_desc.InputSlotClass;
		InstanceDataStepRate = d3d_desc.InstanceDataStepRate;
	}

	input_element_desc(const D3D11_SIGNATURE_PARAMETER_DESC& d3d_parameter_desc, int input_slot) {
		this->SemanticName = new char[NAME_SIZE];
		strcpy(const_cast<LPSTR>(this->SemanticName), d3d_parameter_desc.SemanticName);
		this->SemanticIndex = d3d_parameter_desc.SemanticIndex;
		this->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		this->InputSlot = input_slot;
		this->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		this->InstanceDataStepRate = 0;

		if (d3d_parameter_desc.Mask == 1) {
			if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) this->Format = DXGI_FORMAT_R32_UINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) this->Format = DXGI_FORMAT_R32_SINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) this->Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (d3d_parameter_desc.Mask <= 3) {
			if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) this->Format = DXGI_FORMAT_R32G32_UINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) this->Format = DXGI_FORMAT_R32G32_SINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) this->Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (d3d_parameter_desc.Mask <= 7) {
			if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) this->Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) this->Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) this->Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (d3d_parameter_desc.Mask <= 15) {
			if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) this->Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) this->Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (d3d_parameter_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) this->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}

	~input_element_desc() {
		delete[] SemanticName;
	}

	input_element_desc& operator=(const input_element_desc& d3d_desc) {
		SemanticName = new char[NAME_SIZE];
		strcpy(const_cast<char*>(SemanticName), d3d_desc.SemanticName);

		SemanticIndex = d3d_desc.SemanticIndex;
		Format = d3d_desc.Format;
		InputSlot = d3d_desc.InputSlot;
		AlignedByteOffset = d3d_desc.AlignedByteOffset;
		InputSlotClass = d3d_desc.InputSlotClass;
		InstanceDataStepRate = d3d_desc.InstanceDataStepRate;

		return *this;
	}
};

struct input_layout_data_t {
	input_layout_data_t() : ptr(nullptr), desc() {}
	input_layout_data_t(dx_input_layout ptr, vertex_input_desc desc) :
		ptr{ ptr }, desc{ desc } {}

	dx_input_layout ptr;
	vertex_input_desc desc;
};

struct mesh_t {
	pn::vector<pn::vec3f>		vertices;
	pn::vector<pn::vec4f>		colors;
	pn::vector<pn::vec3f>		normals;
	pn::vector<pn::vec3f>		tangents;
	pn::vector<pn::vec3f>		bitangents;
	pn::vector<pn::vec2f>		uvs;
	pn::vector<pn::vec2f>		uv2s;

	pn::vector<unsigned int>	indices;
	D3D_PRIMITIVE_TOPOLOGY		topology;

	pn::string					name;

	mesh_t()						= default;
	mesh_t(mesh_t&&)				= default;

	mesh_t(const mesh_t&)			= delete;
	mesh_t operator=(const mesh_t&) = delete;

};

struct mesh_buffer_t {
	dx_buffer				vertices;
	dx_buffer				colors;
	dx_buffer				normals;
	dx_buffer				tangents;
	dx_buffer				bitangents;
	dx_buffer				uvs;
	dx_buffer				uv2s;

	dx_buffer				indices;
	unsigned int			index_count;
	D3D_PRIMITIVE_TOPOLOGY	topology;

	pn::rdb::resource_id_t	id;
	pn::string				name;
};

template<typename DxShaderPtrT>
struct shader_data_t {
	DxShaderPtrT			shader;
	dx_shader_reflection	reflection;
};

struct shader_program_t {
	input_layout_data_t				input_layout_data;
	shader_data_t<dx_vertex_shader> vertex_shader_data;
	shader_data_t<dx_pixel_shader>	pixel_shader_data;
};

// --------- GLOBAL STATE -----------

extern dx_device         _device;
extern dx_context        _context;

// ------------------- FUNCTIONS ----------------------

// ---------- STATE ACCESS --------------

dx_device GetDevice();
void      SetDevice(dx_device device);

// ------------ CREATION FUNCTIONS -------------

dx_device				CreateDevice();
dx_swap_chain			CreateSwapChain(DXGI_SWAP_CHAIN_DESC swap_chain_desc);
dx_swap_chain			CreateMainWindowSwapChain(const window_handle hwnd, const application_window_desc awd);

dx_render_target_view	CreateRenderTargetView(dx_texture2d texture);
dx_depth_stencil_view	CreateDepthStencilView(dx_texture2d& depth_stencil_texture);

dx_texture2d			CreateTexture2D(CD3D11_TEXTURE2D_DESC texture_desc, const D3D11_SUBRESOURCE_DATA* initial_data = nullptr);
dx_sampler_state		CreateSamplerState(CD3D11_SAMPLER_DESC sampler_desc);
dx_sampler_state		CreateSamplerState();


vector<mesh_buffer_t>	CreateMeshBuffer(const pn::vector<mesh_t>& mesh);
mesh_buffer_t			CreateMeshBuffer(const mesh_t& mesh);

dx_resource_view        CreateShaderResourceView(dx_resource resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* resource_desc = nullptr);


// -------------- SHADER CREATION -------------

pn::bytes				CompileShader(
	const pn::string& filename,
	const D3D_SHADER_MACRO* defines,
	const pn::string& entry_point,
	const pn::string& shader_type,
	unsigned int flags);

shader_program_t		CompileShaderProgram(const pn::string& filename, const D3D_SHADER_MACRO* defines = nullptr, unsigned int flags = DEFAULT_SHADER_COMPILATION_FLAGS);

template<typename ID3D11ShaderType, typename DeviceShaderFunc>
auto					CreateShader(const pn::bytes& bytes, DeviceShaderFunc CreateShader) {

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

pn::bytes				CompileVertexShader(const pn::string& filename, const D3D_SHADER_MACRO* defines = nullptr, unsigned int flags = DEFAULT_SHADER_COMPILATION_FLAGS);

dx_vertex_shader		CreateVertexShader(const pn::bytes& bytes);
dx_vertex_shader		CreateVertexShader(const pn::string& filename);

pn::bytes				CompilePixelShader(const pn::string& filename, const D3D_SHADER_MACRO* defines = nullptr, unsigned int flags = DEFAULT_SHADER_COMPILATION_FLAGS);

dx_pixel_shader			CreatePixelShader(const pn::bytes& ps_data);
dx_pixel_shader			CreatePixelShader(const pn::string& filename);

input_layout_data_t		CreateInputLayout(const pn::bytes& bytes, const vertex_input_desc& desc);
input_layout_data_t		CreateInputLayout(const pn::bytes& bytes);

std::pair<dx_vertex_shader, input_layout_data_t> 
						CreateVertexShaderAndInputLayout(const pn::string& filename, const vertex_input_desc& desc);

std::pair<dx_vertex_shader, input_layout_data_t> 
						CreateVertexShaderAndInputLayout(const pn::bytes& vs_byte_code, const vertex_input_desc& desc);

std::pair<dx_vertex_shader, input_layout_data_t> 
						CreateVertexShaderAndInputLayout(const pn::string& filename);

// -------------- BUFFER CREATION -------------------

template<typename BufferDataType>
auto					CreateBufferDataDesc(BufferDataType* data, unsigned int pitch = 0, unsigned int slice_pitch = 0) {
	D3D11_SUBRESOURCE_DATA data_desc;
	ZeroMemory(&data_desc, sizeof(decltype(data_desc)));
	data_desc.pSysMem          = data;
	data_desc.SysMemPitch      = pitch;
	data_desc.SysMemSlicePitch = slice_pitch;
	return data_desc;
}

template<typename BufferDataType>
auto					CreateBuffer(BufferDataType* v_data, const size_t n, const D3D11_BIND_FLAG bind_target) {
	// Create buffer description
	CD3D11_BUFFER_DESC buffer_desc(static_cast<unsigned int>(n) * sizeof(BufferDataType), bind_target);

	// Create buffer data
	auto data_desc = CreateBufferDataDesc(v_data);

	dx_buffer buffer;
	auto hr = _device->CreateBuffer(&buffer_desc, &data_desc, &buffer);
	if (FAILED(hr)) {
		LogError("Couldn't create buffer {}: {}", bind_target, pn::ErrMsg(hr));
	}

	return buffer;
}

template<typename VertexDataType>
auto					CreateVertexBuffer(const VertexDataType* v_data, const size_t n) {
	return CreateBuffer(v_data, n, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER);
}

template<typename VertexDataType>
auto					CreateVertexBuffer(const pn::vector<VertexDataType>& v_data) {
	return CreateVertexBuffer(v_data.data(), v_data.size());
}

template<typename ConstantDataType>
auto					CreateConstantBuffer(const ConstantDataType* c_data, const size_t n) {
	static_assert((sizeof(ConstantDataType) % 16) == 0, "Constant Buffer size must be 16-byte aligned");
	return CreateBuffer(c_data, n, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER);
}

template<typename ConstantDataType>
auto					CreateConstantBuffer(const pn::vector<ConstantDataType>& c_data) {
	return CreateConstantBuffer(c_data.data(), c_data.size());
}

template<typename IndexDataType>
auto					CreateIndexBuffer(const IndexDataType* i_data, const size_t n) {
	return CreateBuffer(i_data, n, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER);
}

template<typename IndexDataType>
auto					CreateIndexBuffer(const pn::vector<IndexDataType>& i_data) {
	return CreateIndexBuffer(i_data.data(), i_data.size());
}

// ------------ UTILITY FUNCTIONS -------------

dx_texture2d            GetSwapChainBuffer(dx_swap_chain swap_chain);
dx_context				GetContext(dx_device device);

D3D11_BUFFER_DESC       GetDesc(dx_buffer buffer);
CD3D11_TEXTURE2D_DESC	GetDesc(dx_texture2d texture);

void ClearDepthStencilView(dx_depth_stencil_view view, unsigned int flags, float depth, UINT8 stencil);
void ClearRenderTargetView(dx_render_target_view view, pn::vec4f color);

void SetRenderTarget(dx_render_target_view render_target, dx_depth_stencil_view depth_stencil = nullptr);



// ---------- SHADER REFLECTION ----------------

dx_shader_reflection			GetShaderReflector(const pn::bytes& shader_byte_code);

vertex_input_desc				GetVertexInputDescFromShader(const pn::bytes& vs_byte_code);
vertex_input_desc				GetVertexInputDescFromShader(dx_shader_reflection reflector);

D3D11_SHADER_INPUT_BIND_DESC	GetResourceBindingDesc(dx_shader_reflection reflector, const pn::string& name);

unsigned int					GetShaderResourceStartSlot(dx_shader_reflection reflector, const pn::string& name);

// ----------- VIEWPORT -----------------------

void SetViewport(const int width, const int height, const int top_left_x = 0, const int top_left_y = 0);

D3D11_VIEWPORT GetViewport(int viewport_id = 0);

mat4f GetViewportMatrix(D3D11_VIEWPORT viewport);

dx_render_target_view CreateRenderTargetView(dx_swap_chain swap_chain);

dx_depth_stencil_view CreateDepthStencilView(dx_swap_chain swap_chain);

void SetRenderTargetAndDepthStencilFromSwapChain(
	dx_swap_chain          swap_chain, 
	dx_render_target_view& render_target_view, 
	dx_depth_stencil_view& depth_stencil_view
);

void ResizeRenderTargetViewportCamera(
	unsigned int width, unsigned int height, 
	dx_swap_chain& swap_chain, 
	dx_render_target_view& render_target_view, 
	dx_depth_stencil_view& depth_stencil_view, 
	ProjectionMatrix& camera
);

// --------- SHADER STATE ----------------

void SetVertexShader(dx_vertex_shader shader);
void SetPixelShader(dx_pixel_shader shader);

void SetInputLayout(const input_layout_data_t& layout_desc);

void SetVertexBuffers(const mesh_buffer_t& mesh_buffer);

void SetVSConstant(dx_shader_reflection reflection, const pn::string& buffer_name, const dx_buffer& buffer);
void SetPSConstant(dx_shader_reflection reflection, const pn::string& buffer_name, const dx_buffer& buffer);
void SetProgramConstant(const shader_program_t& program, const pn::string& buffer_name, const dx_buffer& buffer);

void SetVSShaderResource(dx_shader_reflection reflection, const pn::string& resource_name, dx_resource_view& resource_view);
void SetPSShaderResource(dx_shader_reflection reflection, const pn::string& resource_name, dx_resource_view& resource_view);
void SetProgramResource(const shader_program_t& program, const pn::string& resource_name, dx_resource_view& resource_view);

void SetVSSampler(dx_shader_reflection reflection, const pn::string& sampler_name, dx_sampler_state& sampler_state);
void SetPSSampler(dx_shader_reflection reflection, const pn::string& sampler_name, dx_sampler_state& sampler_state);
void SetProgramSampler(const shader_program_t& program, const pn::string& sampler_name, dx_sampler_state& sampler_state);


// ---------- BLENDING -----------------

CD3D11_BLEND_DESC CreateAlphaBlendDesc(const int render_target = 0);
dx_blend_state    CreateBlendState(CD3D11_BLEND_DESC* blend_desc = nullptr);

void SetBlendState();
void SetBlendState(dx_blend_state blend_state);

dx_blend_state GetBlendState();

// --------- DEPTH STENCIL -------------

dx_depth_stencil_state CreateDepthStencilState(CD3D11_DEPTH_STENCIL_DESC* desc = nullptr);

void SetDepthStencilState();
void SetDepthStencilState(dx_depth_stencil_state depth_stencil_state);

dx_depth_stencil_state GetDepthStencilState();

// -------- RASTERIZER -------------

dx_rasterizer_state CreateRasterizerState(CD3D11_RASTERIZER_DESC* desc = nullptr);

void SetRasterizerState();
void SetRasterizerState(dx_rasterizer_state rasterizer_state);

dx_rasterizer_state GetRasterizerState();

// -------- DRAWING FUNCTIONS ---------------

void Draw(const mesh_buffer_t& mesh_buffer);
void DrawIndexed(const mesh_buffer_t& mesh_buffer, unsigned int start_vertex_location = 0, unsigned int base_vertex_location = 0);



} // namespace pn