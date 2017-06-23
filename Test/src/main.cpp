#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>

#include <chrono>

static pn::dx_device device;
static pn::dx_swap_chain swap_chain;
static pn::dx_render_target_view render_target_view;
static pn::dx_depth_stencil_view depth_stencil_view;
static pn::ProjectionMatrix camera;

extern IMGUI_API LRESULT   ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

pn::window_long CALLBACK WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		return true;
	}

	switch (uMsg) {
	case WM_SIZE:
		if (device.Get() != nullptr && wParam != SIZE_MINIMIZED) {
			auto width = (unsigned int) LOWORD(lParam);
			auto height = (unsigned int) HIWORD(lParam);
			LogDebug("Resizing window to width: {}, height: {}", width, height);

			ImGui_ImplDX11_InvalidateDeviceObjects();
			if (render_target_view.Get() != nullptr) {
				render_target_view.ReleaseAndGetAddressOf();
				render_target_view = nullptr;
			}

			swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
			pn::ResizeRenderTargetViewportCamera(device, width, height, swap_chain, render_target_view, depth_stencil_view, camera);

			ImGui_ImplDX11_CreateDeviceObjects(); 
		}
		return 0;
	case WM_CLOSE:
	{
		pn::menu_handle hMenu;
		hMenu = GetMenu(hwnd);
		if (hMenu != NULL) {
			DestroyMenu(hMenu);
		}
		DestroyWindow(hwnd);
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CREATE:
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(pn::instance_handle hInstance, pn::instance_handle hPrevInstance, pn::window_pwstr command_line_args, int nCmdShow) {
	
	// INIT ENVIRONMENT

	pn::CreateConsole();
	pn::InitLogger();
	pn::InitPathUtil();

	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/Test/");
	pn::SetResourceDirectoryName("resource");

	if (hInstance == NULL) {
		hInstance = (pn::instance_handle) GetModuleHandle(NULL);
	}

	auto hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr)) {
		LogError("Couldn't initialize COM: {}", pn::ErrMsg(hr));
	}

	// INIT WINDOWS

	pn::application_window_desc awd;
	awd.h_instance = hInstance;
	awd.width = 1366;
	awd.height = 768;
	awd.fullscreen = false;
	auto h_wnd = pn::CreateApplicationWindow(awd, WindowProc);

	
	// INIT DIRECTX

	device = pn::CreateDevice();
	swap_chain = pn::CreateMainWindowSwapChain(device, h_wnd, awd);

	pn::InitTextureFactory(device);

	pn::SetRenderTargetViewAndDepthStencilFromSwapChain(device, swap_chain, render_target_view, depth_stencil_view);

	auto context = pn::GetContext(device);
	pn::SetViewport(context, awd.width, awd.height);

	// ---------- LOAD RESOURCES ----------------

	auto mesh = pn::LoadMesh(pn::GetResourcePath("plane.fbx"));
	auto mesh_buffer = pn::CreateMeshBuffer(device, mesh);

	auto tex = pn::LoadTexture2D(pn::GetResourcePath("image.png"));

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias = 0.0f;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = -FLT_MAX;
	sampler_desc.MaxLOD = FLT_MAX;
	//sampler_desc.BorderColor = pn::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
	pn::dx_ptr<ID3D11SamplerState> sampler_state;
	auto shr = device->CreateSamplerState(&sampler_desc, sampler_state.GetAddressOf());

	// --------- CREATE SHADER DATA ---------------

	auto vs_byte_code = pn::ReadFile(pn::GetResourcePath("vs.cso"));
	auto vertex_shader = pn::CreateVertexShader(device, vs_byte_code);
	auto input_layout = pn::CreateInputLayout(device, vs_byte_code);

	auto pixel_shader = pn::CreatePixelShader(device, pn::GetResourcePath("ps.cso"));

	struct GlobalConstantBufferData {
		float t = 0.0f;
		float screen_width;
		float screen_height;
		float padding[1];
	};
	GlobalConstantBufferData c;
	c.screen_width = static_cast<float>(awd.width);
	c.screen_height = static_cast<float>(awd.height);
	auto global_constant_buffer = pn::CreateConstantBuffer(device, &c, 1);

	struct InstanceConstantBufferData {
		pn::mat4f model;
		pn::mat4f view;
		pn::mat4f proj;
	};
	InstanceConstantBufferData ic;
	auto instance_constant_buffer = pn::CreateConstantBuffer(device, &ic, 1);

	ic.model = DirectX::XMMatrixTranslation(0.0, 0.0, 4.0);
	ic.view = DirectX::XMMatrixIdentity();

	camera = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(awd.width), static_cast<float>(awd.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	ic.proj = camera.GetMatrix();
	
	// ------ SET UP IMGUI ------------------------------

	ImGui_ImplDX11_Init(h_wnd, device.Get(), context.Get());

	// MAIN LOOP

	ShowWindow(h_wnd, nCmdShow);

	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_col = ImColor(114, 144, 154);
	bool show_edit_matrix = true;

	bool bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
	
	auto prev_time = std::chrono::system_clock::now();
	double time_to_process = 0;
	double total_time = 0;
	const double FPS = 60.0;
	const double FIXED_DT = 1 / FPS;
	while (WM_QUIT != msg.message) {

		// Get and handle input
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// Update the game
		auto current_time = std::chrono::system_clock::now();
		auto dt = std::chrono::duration<double>(current_time - prev_time).count();
		time_to_process += dt;
		total_time += dt;
		prev_time = current_time;

		int u = 0;
		while (time_to_process >= FIXED_DT) {
			// update(FIXED_DT)
			time_to_process -= FIXED_DT;
		}

		// Render

		ImGui_ImplDX11_NewFrame();

		float color[] = { (cos(total_time) + 1)*0.5, (cos(3*total_time) + 1)*0.5, 0.439f, 1.000f };
		context->ClearRenderTargetView(render_target_view.Get(), color);
		context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());

		// Update global uniforms
		c.t += static_cast<float>(dt);
		auto screen_desc = pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
		c.screen_width = static_cast<float>(screen_desc.Width);
		c.screen_height = static_cast<float>(screen_desc.Height);
		context->UpdateSubresource(global_constant_buffer.Get(), 0, nullptr, &c, 0, 0);

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
		auto create_transform_edit = [](const std::string& name, pn::mat4f& matrix, float min, float max) {
			DirectX::XMVECTOR translation, scale, rotation;
			DirectX::XMMatrixDecompose(&scale, &rotation, &translation, matrix);

			ImGui::SliderFloat3((name + " position").c_str(), (float*) &translation, min, max);
			ImGui::SliderFloat3((name + " scale").c_str(), (float*) &scale, 0, max);
			ImGui::SliderFloat4((name + " rotation").c_str(), (float*) &rotation, min, max);
			matrix = DirectX::XMMatrixScalingFromVector(scale) * DirectX::XMMatrixRotationQuaternion(rotation) * DirectX::XMMatrixTranslationFromVector(translation);
		};

		// update world and view
		{
			ImGui::Begin("Edit Cube Matrix", &show_edit_matrix);
			ImGui::Text("Cube");
			create_transform_edit("world", ic.model, -10.0f, 10.0f);
			ImGui::Separator();
			create_transform_edit("view", ic.view, -10.0f, 10.0f);
			ImGui::End();
		}

		// update projection
		float width = camera.GetViewWidth();
		float height = camera.GetViewHeight();
		float fov = camera.GetFov();
		float size = camera.GetOrthographicSize();

		VDBM(&width, 1.0f, 3000.0f);
		VDBM(&height, 1.0f, 3000.0f);
		VDBM(&fov, 1.0f, 180.0f);
		ImGui::SliderFloat("size", &size, 0.001f, 1.0f, "%.3f", 2.0f);
		//VDBM(&size, 0.001f, 1.0f);

		camera.SetOrthographicSize(size);
		camera.SetFov(fov);
		camera.SetViewWidth(width);
		camera.SetViewHeight(height);

		bool toggle_proj = ImGui::Button("Toggle");
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

		context->DrawIndexed(mesh[0].indices.size(), 0, 0);
		ImGui::Render();
		auto hr = swap_chain->Present(1, 0);
		if (FAILED(hr)) {
			LogError("Swap chain present error: ", pn::ErrMsg(hr));
		}
	}

	// Shutdown
	pn::CloseLogger();
}