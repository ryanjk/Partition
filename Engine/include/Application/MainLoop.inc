#pragma once

#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\TextureLoadUtil.h>

#include <Utilities\Logging.h>

#include <Input\Input.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <Application\Global.h>

#include <chrono>

// --------- VARIABLES -------------
pn::dx_device				device;
pn::dx_swap_chain			swap_chain;
pn::dx_render_target_view	render_target_view;
pn::dx_depth_stencil_view	depth_stencil_view;
pn::ProjectionMatrix		camera;

// -------- USER-DEFINED MAIN LOOP FUNCTIONS -----------

void Init();
void GetInput();
void Update(const float dt);
void Render();

// ----------- LOOP FUNCTIONS --------------
extern IMGUI_API LRESULT	ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

pn::window_long CALLBACK	WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		//return true;
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

	case WM_KEYDOWN:
		if (wParam < 256) {
			pn::input::SetKeyState(wParam, true);
		}
		break;

	case WM_KEYUP:
		if (wParam < 256) {
			pn::input::SetKeyState(wParam, false);
		}
		break;

	case WM_LBUTTONDOWN:
		pn::input::SetLeftMouse(true);
		break;

	case WM_LBUTTONUP:
		pn::input::SetLeftMouse(false);
		break;

	case WM_RBUTTONDOWN:
		pn::input::SetRightMouse(true);
		break;

	case WM_RBUTTONUP:
		pn::input::SetRightMouse(false);
		break;

	case WM_MBUTTONDOWN:
		pn::input::SetMiddleMouse(true);
		break;

	case WM_MBUTTONUP:
		pn::input::SetMiddleMouse(false);
		break;
		
	case WM_MOUSEWHEEL:
		pn::input::IncrementMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f);
		break;

	case WM_MOUSEMOVE:
		pn::input::SetMousePos({ (signed short) (lParam), (signed short) (lParam >> 16) });
		break;
	
	case WM_CHAR:
		pn::input::AddInputCharacter((unsigned short) wParam);
		break;
	
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// PROGRAM ENTRY POINT
int WINAPI					wWinMain(pn::instance_handle hInstance, pn::instance_handle hPrevInstance, pn::window_pwstr command_line_args, int nCmdShow) {

	// INIT ENVIRONMENT

	pn::CreateConsole();
	pn::InitLogger();
	pn::InitPathUtil();
	pn::input::InitInput();

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

	bool	show_test_window	= true;
	bool	show_another_window = false;
	ImVec4	clear_col			= ImColor(114, 144, 154);
	bool	show_edit_matrix	= true;
	bool	show_main_menu		= true;

	// ------- USER-DEFINED INIT CALL -----------------
	Init();

	// ----- DISPLAY WINDOW -----------
	ShowWindow(h_wnd, nCmdShow);

	// ----- MAIN LOOP------------

	bool bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	auto prev_time = std::chrono::system_clock::now();
	float time_to_process = 0;
	pn::app::FPS = 60.0f;
	pn::app::FIXED_DT = 1 / pn::app::FPS;
	while (WM_QUIT != msg.message) {

		// BEGINNING OF FRAME CALLS

		// Get and handle input
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		auto input_chars = pn::input::GetInputCharacters();
		if (pn::Size(input_chars) > 0) {
			Log("{}", pn::input::GetInputCharacters());
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

		// END OF FRAME CALLS
		pn::input::ClearInputCharacters();
	}

	// Shutdown
	pn::CloseLogger();
}
