#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <chrono>

static pn::dx_device device;
static pn::dx_swap_chain swap_chain;
static pn::dx_render_target_view render_target_view;
static pn::dx_depth_stencil_view depth_stencil_view;
static pn::ProjectionMatrix camera;

pn::window_long CALLBACK WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	/*if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		return true;
	} */

	switch (uMsg) {
	case WM_SIZE:
		if (device.Get() != nullptr && wParam != SIZE_MINIMIZED) {
			auto width = (unsigned int) LOWORD(lParam);
			auto height = (unsigned int) HIWORD(lParam);
			LogDebug("Resizing window to width: {}, height: {}", width, height);

			//ImGui_ImplDX11_InvalidateDeviceObjects();
			if (render_target_view.Get() != nullptr) {
				render_target_view.ReleaseAndGetAddressOf();
				render_target_view = nullptr;
			}

			swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
			pn::ResizeRenderTargetViewportCamera(device, width, height, swap_chain, render_target_view, depth_stencil_view, camera);

			//ImGui_ImplDX11_CreateDeviceObjects(); 
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

int PARTITION_MAIN(command_line_args) {
	
	// INIT ENVIRONMENT

	pn::CreateConsole();
	pn::InitLogger();
	pn::InitPathUtil();

	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/Test/");
	pn::SetResourceDirectoryName("resource");

	if (hInstance == NULL) {
		hInstance = (pn::instance_handle) GetModuleHandle(NULL);
	}

	// INIT WINDOW

	pn::application_window_desc awd;
	awd.h_instance = hInstance;
	awd.width = 1366;
	awd.height = 768;
	awd.fullscreen = false;
	auto h_wnd = pn::CreateApplicationWindow(awd, WindowProc);
	
	// INIT DIRECTX

	device = pn::CreateDevice();
	swap_chain = pn::CreateMainWindowSwapChain(device, h_wnd, awd);

	pn::SetRenderTargetViewAndDepthStencilFromSwapChain(device, swap_chain, render_target_view, depth_stencil_view);

	auto context = pn::GetContext(device);
	pn::SetViewport(context, awd.width, awd.height);

	// LOAD RESOURCES

	auto mesh = pn::LoadMesh(pn::GetResourcePath("torus.fbx"));
	auto mesh_buffer = pn::CreateMeshBuffer(device, mesh);

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
	
	// MAIN LOOP

	ShowWindow(h_wnd, nCmdShow);

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

		auto hr = swap_chain->Present(1, 0);
		if (FAILED(hr)) {
			LogError("Swap chain present error: ", pn::ErrMsg(hr));
		}
	}

	// Shutdown
	pn::CloseLogger();
}