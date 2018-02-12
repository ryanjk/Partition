#include <Graphics\RenderSystem.h>
#include <Application\Global.h>

namespace pn {

// ----- GLOBALS ------

pn::dx_swap_chain				SWAP_CHAIN;

pn::cbuffer<global_constants_t>	global_constants;
pn::cbuffer<camera_constants_t>	camera_constants;
pn::cbuffer<model_constants_t>	model_constants;

pn::dx_render_target_view		DISPLAY_RENDER_TARGET;
pn::dx_depth_stencil_view		DISPLAY_DEPTH_STENCIL;
pn::camera_t			        MAIN_CAMERA;

shader_program_t* CURRENT_SHADER;


// ----- FUNCTIONS -----

void InitRenderSystem(const window_handle h_wnd, const application_window_desc awd) {
	SWAP_CHAIN = pn::CreateMainWindowSwapChain(h_wnd, awd);
	pn::SetViewport(awd.width, awd.height);

	pn::SetRenderTargetAndDepthStencilFromSwapChain(SWAP_CHAIN, DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);

	pn::InitializeCBuffer(model_constants);
	
	pn::InitializeCBuffer(global_constants);
	UpdateGlobalConstantCBuffer();

	pn::InitializeCBuffer(camera_constants);
	MAIN_CAMERA.transform = transform_t{};
	MAIN_CAMERA.projection_matrix = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 100.0f,
		70.0f, 0.1f
	};
	UpdateCameraConstantCBuffer(MAIN_CAMERA);

	{
		CD3D11_BLEND_DESC desc(D3D11_DEFAULT);
		auto blend_state = CreateBlendState(&desc);
		SetBlendState(blend_state);
	}

	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);
		auto rasterizer_state = CreateRasterizerState(&desc);
		SetRasterizerState(rasterizer_state);
	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		auto depth_stencil_state = CreateDepthStencilState(&desc);
		SetDepthStencilState(depth_stencil_state);
	}
}

void UpdateGlobalConstantCBuffer() {
	// Update global uniforms
	global_constants.data.t             += static_cast<float>(pn::app::dt);
	global_constants.data.screen_width  = static_cast<float>(pn::app::window_desc.width);
	global_constants.data.screen_height = static_cast<float>(pn::app::window_desc.height);

	auto screen_desc = pn::GetDesc(pn::GetSwapChainBuffer(SWAP_CHAIN));
	global_constants.data.screen_width = static_cast<float>(screen_desc.Width);
	global_constants.data.screen_height = static_cast<float>(screen_desc.Height);

	UpdateBuffer(global_constants);
}

void UpdateCameraConstantCBuffer(const camera_t& camera) {
	// Update camera buffer
	camera_constants.data.view = Inverse(TransformToMatrix(camera.transform));
	camera_constants.data.inv_view = TransformToMatrix(camera.transform);
	camera_constants.data.proj = camera.projection_matrix.GetMatrix();
	camera_constants.data.inv_proj = Inverse(camera.projection_matrix.GetMatrix());
	camera_constants.data.inv_proj_view = Inverse(camera_constants.data.view * camera_constants.data.proj);

	UpdateBuffer(camera_constants);
}

void UpdateModelConstantCBuffer(const transform_t& transform) {
	model_constants.data.model = LocalToWorldMatrix(transform);
	model_constants.data.model_view = model_constants.data.model * camera_constants.data.view;
	model_constants.data.model_view_inverse_transpose = pn::Transpose(pn::Inverse(model_constants.data.model * camera_constants.data.view));
	model_constants.data.mvp = model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;

	UpdateBuffer(model_constants);
}

// ----- STATE ACCESS -------

void ResetRenderTarget() {
	pn::SetRenderTarget(DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);
}

void ClearDepthStencil() {
	pn::ClearDepthStencilView(DISPLAY_DEPTH_STENCIL, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void ClearDisplay(const pn::vec4f color) {
	pn::ClearRenderTargetView(DISPLAY_RENDER_TARGET, color);
}

void SetShaderProgram(shader_program_t& shader_program) {
	pn::SetInputLayout(shader_program.input_layout_data);
	pn::SetVertexShader(shader_program.vertex_shader_data.shader);
	pn::SetPixelShader(shader_program.pixel_shader_data.shader);
	CURRENT_SHADER = &shader_program;
}

void SetStandardShaderProgram(shader_program_t& shader_program) {
	SetShaderProgram(shader_program);
	SetProgramConstant("global_constants", global_constants);
	SetProgramConstant("camera_constants", camera_constants);
	SetProgramConstant("model_constants", model_constants);
}

void ClearShaderProgram() {
	CURRENT_SHADER = nullptr;
	_context->PSSetShader(nullptr, nullptr, 0);
	_context->VSSetShader(nullptr, nullptr, 0);
	_context->IASetInputLayout(nullptr);
}

void SetVertexBuffers(const mesh_buffer_t& mesh_buffer) {
	const input_layout_data_t& layout = CURRENT_SHADER->input_layout_data;
	const auto NUM_PARAMETERS = layout.desc.size();

	pn::vector<ID3D11Buffer*> vertex_buffers;
	Reserve(vertex_buffers, NUM_PARAMETERS);

	pn::vector<unsigned int> strides;
	Reserve(strides, NUM_PARAMETERS);

	pn::vector<unsigned int> offsets;
	Reserve(offsets, NUM_PARAMETERS);

	for (size_t i = 0; i < layout.desc.size(); ++i) {
		const auto& el = layout.desc[i];
		unsigned int index = layout.desc[i].SemanticIndex;
		std::string type = el.SemanticName;
		if (type == "POSITION") {
			PushBack(vertex_buffers, mesh_buffer.vertices.Get());
			PushBack(strides, sizeof(pn::vec3f));
			PushBack(offsets, 0);
		}
		else if (type == "NORMAL") {
			PushBack(vertex_buffers, mesh_buffer.normals.Get());
			PushBack(strides, sizeof(pn::vec3f));
			PushBack(offsets, 0);
		}
		else if (type == "TEXCOORD") {
			if (index == 0) {
				PushBack(vertex_buffers, mesh_buffer.uvs.Get());
				PushBack(strides, sizeof(pn::vec2f));
				PushBack(offsets, 0);
			}
			else if (index == 1) {
				PushBack(vertex_buffers, mesh_buffer.uv2s.Get());
				PushBack(strides, sizeof(pn::vec2f));
				PushBack(offsets, 0);
			}
			else {
				LogError("TEXCOORD with index {} not implemented", index);
			}
		}
		else if (type == "TANGENT") {
			if (index == 0) {
				PushBack(vertex_buffers, mesh_buffer.tangents.Get());
				PushBack(strides, sizeof(pn::vec3f));
				PushBack(offsets, 0);
			}
			else if (index == 1) {
				PushBack(vertex_buffers, mesh_buffer.bitangents.Get());
				PushBack(strides, sizeof(pn::vec3f));
				PushBack(offsets, 0);
			}
			else {
				LogError("TANGENT with index {} not implemented", index);
			}
		}
		else if (type == "COLOR") {
			PushBack(vertex_buffers, mesh_buffer.colors.Get());
			PushBack(strides, sizeof(pn::vec4f));
			PushBack(offsets, 0);
		}
		else {
			LogError("Unknown parameter type '{}' in MeshBuffer", type);
		}
	}

	_context->IASetVertexBuffers(0, vertex_buffers.size(), const_cast<const pn::vector<ID3D11Buffer*>&>(vertex_buffers).data(), strides.data(), offsets.data());
	_context->IASetIndexBuffer(mesh_buffer.indices.Get(), DXGI_FORMAT_R32_UINT, 0);
	_context->IASetPrimitiveTopology(mesh_buffer.topology);
}

void SetVertexBuffersScreen() {
	auto inverse_view = Inverse(camera_constants.data.view);
	auto inverse_projection = Inverse(camera_constants.data.proj);
	const mat4f corners{
		vec4f(-1.f, 1.f , 1.f, 1.f),
		vec4f(1.f , 1.f , 1.f, 1.f),
		vec4f(-1.f, -1.f, 1.f, 1.f),
		vec4f(1.f , -1.f, 1.f, 1.f)
	};

	{
		const vec3f screen_vertex_data[4] = {
			vec3f(corners[0].xy(),0),
			vec3f(corners[1].xy(),0),
			vec3f(corners[2].xy(),0),
			vec3f(corners[3].xy(),0)
		};

		auto screen_vertex_buffer = CreateVertexBuffer(screen_vertex_data, 4);
		ID3D11Buffer* buffers[1] = { screen_vertex_buffer.Get() };
		UINT strides[1] = { sizeof(vec3f) };
		UINT offsets[1] = { 0 };
		_context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
	}

	/*
	Comment this out for now -- building the corners of the view frustum on the back plane
	{
		auto rcorners = corners * inverse_projection;
		rcorners = mat4f(
			rcorners[0] / rcorners[0].w,
			rcorners[1] / rcorners[1].w,
			rcorners[2] / rcorners[2].w,
			rcorners[3] / rcorners[3].w
		);
		rcorners = mat4f(
			vec4f(rcorners[0].xyz(), 0.0f),
			vec4f(rcorners[1].xyz(), 0.0f),
			vec4f(rcorners[2].xyz(), 0.0f),
			vec4f(rcorners[3].xyz(), 0.0f)
		);
		rcorners = rcorners * inverse_view;
		const vec3f screen_vertex_data[4] = {
			rcorners[0].xyz(),
			rcorners[1].xyz(),
			rcorners[2].xyz(),
			rcorners[3].xyz()
		};

		auto screen_vertex_buffer = CreateVertexBuffer(screen_vertex_data, 4);
		ID3D11Buffer* buffers[1] = { screen_vertex_buffer.Get() };
		UINT strides[1] = { sizeof(vec3f) };
		UINT offsets[1] = { 0 };
		_context->IASetVertexBuffers(1, 1, buffers, strides, offsets);
	}
	*/
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void ClearVertexBuffers() {
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
}

void SetProgramConstant(const pn::string& buffer_name, const dx_buffer& buffer) {
	assert(CURRENT_SHADER != nullptr);
	SetProgramConstant(*CURRENT_SHADER, buffer_name, buffer);
}

void SetProgramResource(const pn::string& resource_name, dx_resource_view& resource_view) {
	assert(CURRENT_SHADER != nullptr);
	SetProgramResource(*CURRENT_SHADER, resource_name, resource_view);
}

void SetProgramSampler(const pn::string& sampler_name, dx_sampler_state& sampler_state) {
	assert(CURRENT_SHADER != nullptr);
	SetProgramSampler(*CURRENT_SHADER, sampler_name, sampler_state);
}

void SetAlphaBlend(bool on, int num_render_targets) {
	auto state = GetBlendState();
	CD3D11_BLEND_DESC desc{};
	state->GetDesc(&desc);
	
	for (int i = 0; i < num_render_targets; ++i) {
		desc.RenderTarget[i].BlendEnable = on;
		desc.RenderTarget[i].SrcBlend    = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[i].DestBlend   = D3D11_BLEND_INV_SRC_ALPHA;
	}

	auto new_state = CreateBlendState(&desc);
	SetBlendState(new_state);
}

void SetDepthTest(bool on) {
	auto state = GetDepthStencilState();
	CD3D11_DEPTH_STENCIL_DESC desc;
	state->GetDesc(&desc);

	desc.DepthEnable = on;

	auto new_state = CreateDepthStencilState(&desc);
	SetDepthStencilState(new_state);
}

void SetWireframeMode(bool on) {
	auto state = GetRasterizerState();
	CD3D11_RASTERIZER_DESC desc;
	state->GetDesc(&desc);

	desc.FillMode =  on ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

	auto new_state = CreateRasterizerState(&desc);
	SetRasterizerState(new_state);
}

}