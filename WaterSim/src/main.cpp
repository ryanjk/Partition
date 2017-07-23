#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>
#include <Graphics\Uniform.h>

#include <Utilities\Logging.h>
#include <Utilities\frame_string.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <Component\transform_t.h>

#include <chrono>

#include <Application\MainLoop.inc>

// -- Uniform buffer data definitions ----

struct alignas(16) global_contants {
	float t = 0.0f;
	float screen_width;
	float screen_height;
};

struct camera_constants {
	pn::mat4f view;
	pn::mat4f proj;
};

struct instance_constants {
	pn::mat4f model;
};

struct alignas(16) directional_light {
	pn::vec3f direction;
	float intensity;
};

struct alignas(16) wave {
	float A;
	float L;
	float w;
	float q;
	pn::vec2f d;
};

using pn::uniform;
using pn::uniform_array;

#define N_WAVES 1
uniform<global_contants>		c;
uniform<camera_constants>		camera_buffer;
uniform<instance_constants>		ic;
uniform<directional_light>		dl;
uniform_array<wave, N_WAVES>	wb;

// --- wave instance data ----
pn::transform_t					wave_t;
pn::vector<pn::mesh_buffer_t>	wave_mesh_buffer;
pn::dx_vertex_shader			wave_vs;
pn::dx_pixel_shader				wave_ps;
pn::input_layout_desc			wave_input_layout;

pn::linear_allocator frame_alloc(1024 * 1024);

// ---- misc d3d11 state -----
pn::dx_blend_state blend_state;

void Init() {

	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/");
	pn::SetResourceDirectoryName("Resources");

	// ---------- LOAD RESOURCES ----------------

	auto mesh			= pn::LoadMesh(pn::GetResourcePath("water.fbx"));
	wave_mesh_buffer	= pn::CreateMeshBuffer(device, mesh);

	// ------- SET BLENDING STATE ------------

	blend_state		= pn::CreateBlendState(device);
	pn::SetBlendState(device, blend_state);

	// --------- CREATE SHADER DATA ---------------

	auto vs_byte_code	= pn::ReadFile(pn::GetResourcePath("water_vs.cso"));
	wave_vs				= pn::CreateVertexShader(device, vs_byte_code);
	wave_input_layout	= pn::CreateInputLayout(device, vs_byte_code);

	wave_ps				= pn::CreatePixelShader(device, pn::GetResourcePath("water_ps.cso"));

	c.data.screen_width		= static_cast<float>(pn::app::window_desc.width);
	c.data.screen_height	= static_cast<float>(pn::app::window_desc.height);

	Initialize(device, c);
	Initialize(device, camera_buffer);
	Initialize(device, ic);
	Initialize(device, dl);
	Initialize(device, wb);

	wave_t.position = { 0, 0, 15 };
	wave_t.scale	= { 1, 1, 1 };
	wave_t.rotation = { 0.698f, 0.465f, 0.f };
	ic.data.model	= TransformToSRT(wave_t);

	camera_buffer.data.view = pn::mat4f::Identity;

	dl.data.direction = pn::vec3f(0.0f, 0.0f, 1.0f);
	dl.data.intensity = 1.0f;

	for (int i = 0; i < N_WAVES; ++i) {
		wb.data[i].A = 0.615f;
		wb.data[i].L = 5.615;
		wb.data[i].w = 0.615;
		wb.data[i].q = 0;
		wb.data[i].d = { 0.68f, 0.735f };
	}

	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	camera_buffer.data.proj = camera.GetMatrix();

	// --------- INIT CUSTOM ALLOCATORS -----------
	pn::frame_string::SetFrameAllocator(&frame_alloc);
}

void Update(const float dt) {}

void Render() {
	auto context = pn::GetContext(device);

	// Update global uniforms
	c.data.t				+= static_cast<float>(pn::app::dt);
	auto screen_desc		= pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
	c.data.screen_width		= static_cast<float>(screen_desc.Width);
	c.data.screen_height	= static_cast<float>(screen_desc.Height);

	// Set render target backbuffer color
	float color[] = { 0.0f, 0.0f, 0.0f, 1.000f };
	context->ClearRenderTargetView(render_target_view.Get(), color);
	context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

	// update uniform buffers that are shared across shaders
	UpdateBuffer(context, c);
	UpdateBuffer(context, camera_buffer);

// ------ BEGIN WATER

	// set vertex buffer
	auto& cmesh_buffer = wave_mesh_buffer[0];
	pn::SetContextVertexBuffers(context, wave_input_layout, cmesh_buffer);
	context->IASetInputLayout(wave_input_layout.ptr.Get());
	context->IASetIndexBuffer(cmesh_buffer.indices.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(cmesh_buffer.topology);

	// set shader
	context->VSSetShader(wave_vs.Get(), nullptr, 0);
	context->PSSetShader(wave_ps.Get(), nullptr, 0);

	// update wave
	for (int i = 0; i < N_WAVES; ++i) {
		pn::string w_id = pn::string("w") + std::to_string(i);
		ImGui::SliderFloat( (w_id + " amp").c_str(), &wb.data[i].A, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " L").c_str(), &wb.data[i].L, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " w").c_str(), &wb.data[i].w, 0.0f, 10.0f);
		ImGui::SliderFloat((w_id + " Q").c_str(), &wb.data[i].q, 0.0f, 3.0f);
		ImGui::SliderFloat2((w_id + " d").c_str(), &(wb.data[i].d.x), -1.0f, 1.0f);
		wb.data[i].d = (wb.data[i].d == pn::vec2f::Zero) ? pn::vec2f::Zero : pn::Normalize(wb.data[i].d);
	}

	// update model matrix
	ImGui::SliderFloat3("position", &wave_t.position.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("rotation", &wave_t.rotation.x, -pn::TWOPI, pn::TWOPI);
	ic.data.model = TransformToSRT(wave_t);

	// update directional light
	ImGui::SliderFloat3("light dir", &dl.data.direction.x, -1.0f, 1.0f);
	ImGui::SliderFloat("light power", &dl.data.intensity, 0.0f, 10.0f);
	dl.data.direction = dl.data.direction == pn::vec3f::Zero ? pn::vec3f::Zero : pn::Normalize(dl.data.direction);

	// send updates to uniform buffers
	UpdateBuffer(context, ic);
	UpdateBuffer(context, dl);
	UpdateBuffer(context, wb);

	// set constant buffers in shaders
	context->VSSetConstantBuffers(0, 1, c.buffer.GetAddressOf());
	context->VSSetConstantBuffers(1, 1, camera_buffer.buffer.GetAddressOf());
	context->VSSetConstantBuffers(2, 1, ic.buffer.GetAddressOf());
	context->VSSetConstantBuffers(3, 1, wb.buffer.GetAddressOf());

	context->PSSetConstantBuffers(0, 1, c.buffer.GetAddressOf());
	context->VSSetConstantBuffers(1, 1, camera_buffer.buffer.GetAddressOf());
	context->PSSetConstantBuffers(2, 1, ic.buffer.GetAddressOf());
	context->PSSetConstantBuffers(3, 1, dl.buffer.GetAddressOf());

	pn::DrawIndexed(context, wave_mesh_buffer[0]);

// ----- END WATER

}

void MainLoopBegin() {
	frame_alloc.Release();
}

void MainLoopEnd() {}