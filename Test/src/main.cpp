#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <chrono>

#include <Application\MainLoop.inc>

struct alignas(16) GlobalConstantBufferData {
	float t = 0.0f;
	float screen_width;
	float screen_height;
};
GlobalConstantBufferData c;

struct InstanceConstantBufferData {
	pn::mat4f model;
	pn::mat4f view;
	pn::mat4f proj;
};
InstanceConstantBufferData ic;

pn::dx_buffer global_constant_buffer;
pn::dx_buffer instance_constant_buffer;

pn::dx_vertex_shader vertex_shader;
pn::input_layout_desc input_layout;
pn::dx_pixel_shader pixel_shader;

pn::vector<pn::mesh_buffer_t> mesh_buffer;

pn::vec3f pos(0, 0, 4);
pn::vec3f scale(1, 1, 1);
pn::vec3f rot(0, 0, 0);

pn::texture_t tex;
pn::dx_sampler_state sampler_state;

pn::linear_allocator frame_alloc(1024 * 1024);

void Init() {
	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/Test/");
	pn::SetResourceDirectoryName("resource");

	// ---------- LOAD RESOURCES ----------------

	auto mesh = pn::LoadMesh(pn::GetResourcePath("monkey.fbx"));
	mesh_buffer = pn::CreateMeshBuffer(device, mesh);

	tex = pn::LoadTexture2D(pn::GetResourcePath("image.png"));
	sampler_state = pn::CreateSamplerState(device);


	// --------- CREATE SHADER DATA ---------------

	auto vs_byte_code = pn::ReadFile(pn::GetResourcePath("vs.cso"));
	vertex_shader = pn::CreateVertexShader(device, vs_byte_code);
	input_layout = pn::CreateInputLayout(device, vs_byte_code);

	pixel_shader = pn::CreatePixelShader(device, pn::GetResourcePath("ps.cso"));

	c.screen_width = static_cast<float>(pn::app::window_desc.width);
	c.screen_height = static_cast<float>(pn::app::window_desc.height);

	global_constant_buffer = pn::CreateConstantBuffer(device, &c, 1);
	instance_constant_buffer = pn::CreateConstantBuffer(device, &ic, 1);

	ic.model = pn::SRTMatrix(scale, rot, pos);
	ic.view = pn::mat4f::Identity;

	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	ic.proj = camera.GetMatrix();
}

void Update(const float dt) {}

void Render() {
	auto context = pn::GetContext(device);

	// Update global uniforms
	c.t += static_cast<float>(pn::app::dt);
	auto screen_desc = pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
	c.screen_width = static_cast<float>(screen_desc.Width);
	c.screen_height = static_cast<float>(screen_desc.Height);
	context->UpdateSubresource(global_constant_buffer.Get(), 0, nullptr, &c, 0, 0);

	// Set render target backbuffer color
	float color[] = { 
		(cos(pn::app::time_since_application_start) + 1)*0.5, 
		(cos(3 * pn::app::time_since_application_start) + 1)*0.5,
		0.439f, 
		1.000f };
	context->ClearRenderTargetView(render_target_view.Get(), color);
	context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// set vertex buffer
	auto& cmesh_buffer = mesh_buffer[0];
	pn::SetContextVertexBuffers(context, input_layout, cmesh_buffer);
	context->IASetInputLayout(input_layout.ptr.Get());
	context->IASetIndexBuffer(cmesh_buffer.indices.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(cmesh_buffer.topology);

	// set shader
	context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	context->PSSetShader(pixel_shader.Get(), nullptr, 0);

	// update instance uniforms

	// update world and view
	ImGui::SliderFloat3("position", &pos.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("rotation", &rot.x, -pn::TWOPI, pn::TWOPI);
	ic.model = pn::SRTMatrix(scale, rot, pos);

	// update projection
	float width = camera.GetViewWidth();
	float height = camera.GetViewHeight();
	float fov = camera.GetFov();
	float size = camera.GetOrthographicSize();

	VDBM(&width, 1.0f, 3000.0f);
	VDBM(&height, 1.0f, 3000.0f);
	VDBM(&fov, 1.0f, 180.0f);
	ImGui::SliderFloat("size", &size, 0.001f, 1.0f, "%.3f", 2.0f);

	camera.SetOrthographicSize(size);
	camera.SetFov(fov);
	camera.SetViewWidth(width);
	camera.SetViewHeight(height);

	bool toggle_proj = ImGui::Button("Toggle Projection");
	if (toggle_proj) {
		auto cur = camera.GetProjectionType();
		camera.SetProjectionType(cur == pn::ProjectionType::ORTHOGRAPHIC ? pn::ProjectionType::PERSPECTIVE : pn::ProjectionType::ORTHOGRAPHIC);
	}

	ic.proj = camera.GetMatrix();

	// send updates to constant buffer
	context->UpdateSubresource(instance_constant_buffer.Get(), 0, nullptr, &ic, 0, 0);

	// set constant buffers in shaders
	context->VSSetConstantBuffers(0, 1, global_constant_buffer.GetAddressOf());
	context->VSSetConstantBuffers(1, 1, instance_constant_buffer.GetAddressOf());

	context->PSSetConstantBuffers(0, 1, global_constant_buffer.GetAddressOf());

	// update shader textures
	context->PSSetShaderResources(0, 1, tex.resource_view.GetAddressOf());
	context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());

	context->DrawIndexed(mesh_buffer[0].index_count, 0, 0);
}

void MainLoopBegin() {
	frame_alloc.Release();
}

void MainLoopEnd() {}