#pragma once

#include <Graphics\Window.h>
#include <Graphics\DirectX.h>

#include <Utilities\Logging.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <Application\Global.h>

#include <chrono>

// -------- USER-DEFINED MAIN LOOP FUNCTIONS -----------

extern void Init();
extern void Update(const float dt);
extern void Render();

pn::dx_device				device;
pn::dx_swap_chain			swap_chain;
pn::dx_render_target_view	render_target_view;
pn::dx_depth_stencil_view	depth_stencil_view;
pn::ProjectionMatrix		camera;

extern IMGUI_API LRESULT	ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

pn::window_long CALLBACK	WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		return true;
	}

	switch (uMsg) {
	case WM_SIZE:
		if (device.Get() != nullptr && wParam != SIZE_MINIMIZED) {
			auto width = (unsigned int) LOWORD(lParam);
			auto height = (unsigned int) HIWORD(lParam);

			pn::app::window_desc.width = width;
			pn::app::window_desc.height = height;

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

int WINAPI					wWinMain(pn::instance_handle hInstance, pn::instance_handle hPrevInstance, pn::window_pwstr command_line_args, int nCmdShow) {

	// INIT ENVIRONMENT

	pn::CreateConsole();
	pn::InitLogger();
	pn::InitPathUtil();

	if (hInstance == NULL) {
		hInstance = (pn::instance_handle) GetModuleHandle(NULL);
	}

	auto hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr)) {
		LogError("Couldn't initialize COM: {}", pn::ErrMsg(hr));
	}

	// INIT WINDOWS

	auto& awd = pn::app::window_desc;
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

	// ------ SET UP IMGUI ------------------------------

	ImGui_ImplDX11_Init(h_wnd, device.Get(), context.Get());
	pn::gui::InitEditorUI();

	bool	show_test_window = true;
	bool	show_another_window = false;
	ImVec4	clear_col = ImColor(114, 144, 154);
	bool	show_edit_matrix = true;
	bool	show_main_menu = true;

	// ------- USER-DEFINED INIT CALL -----------------
	Init();

	ShowWindow(h_wnd, nCmdShow);
	// MAIN LOOP

	bool bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	auto prev_time = std::chrono::system_clock::now();
	float time_to_process = 0;
	pn::app::FPS = 60.0f;
	pn::app::FIXED_DT = 1 / pn::app::FPS;
	while (WM_QUIT != msg.message) {

		// Get and handle input
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// UPDATE
		auto current_time = std::chrono::system_clock::now();
		pn::app::dt = std::chrono::duration<float>(current_time - prev_time).count();
		time_to_process += pn::app::dt;
		pn::app::time_since_application_start += pn::app::dt;
		prev_time = current_time;

		while (time_to_process >= pn::app::FIXED_DT) {
			Update(pn::app::FIXED_DT);
			time_to_process -= pn::app::FIXED_DT;
		}

		// RENDER

		// Start new frame
		ImGui_ImplDX11_NewFrame();

		// Draw main menu
		show_main_menu = ImGui::Button("Show main menu") ? !show_main_menu : show_main_menu;
		pn::gui::SetMainMenuVisible(show_main_menu);
		pn::gui::DrawMainMenu(awd.width);

		// USER-DEFINED RENDER CALL
		Render();

		ImGui::Render();
		auto hr = swap_chain->Present(1, 0);
		if (FAILED(hr)) {
			LogError("Swap chain present error: ", pn::ErrMsg(hr));
		}
	}

	// Shutdown
	pn::CloseLogger();
}
