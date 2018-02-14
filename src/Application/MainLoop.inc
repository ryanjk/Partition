#pragma once

#include <stdafx.h>
#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\RenderSystem.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\CBuffer.h>
#include <Graphics\DebugDraw.h>

#include <Utilities\Logging.h>
#include <Utilities\Memory.h>

#include <Input\Input.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>

#include <Application\Global.h>

#include <chrono>

using namespace pn;
using namespace pn::input;

pn::window_handle h_wnd;

// -------- USER-DEFINED MAIN LOOP FUNCTIONS -----------

void Init();
void Update();      // Called once per frame
void FixedUpdate(); // Called at fixed rate independent of frame-rate
void Render();
void Close();
void Resize();

void MainLoopBegin();
void MainLoopEnd();

// ----------- LOOP FUNCTIONS --------------
extern IMGUI_API LRESULT	ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

pn::window_long CALLBACK	WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		//return true;
	}

	switch (uMsg) {
	case WM_SIZE:
		if (GetDevice() != nullptr && wParam != SIZE_MINIMIZED) {
			auto width = (unsigned int) LOWORD(lParam);
			auto height = (unsigned int) HIWORD(lParam);

			pn::app::window_desc.width  = width;
			pn::app::window_desc.height = height;

			LogDebug("Resizing window to width: {}, height: {}", width, height);

			if (DISPLAY_RENDER_TARGET.Get() != nullptr) {
				DISPLAY_RENDER_TARGET.ReleaseAndGetAddressOf();
			}

			// Update swap chain
			SWAP_CHAIN->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
			
			// Update render target and depth buffer
			DISPLAY_RENDER_TARGET = CreateRenderTargetView(SWAP_CHAIN);
			DISPLAY_DEPTH_STENCIL = CreateDepthStencilView(SWAP_CHAIN);

			// Update viewport
			pn::SetViewport(width, height);

			// Update camera
			MAIN_CAMERA.projection_matrix.SetViewWidth(static_cast<float>(width));
			MAIN_CAMERA.projection_matrix.SetViewHeight(static_cast<float>(height));

			// User-defined resize
			Resize();
		}
		return 0;
	case WM_SETCURSOR:
	{
		if (input::IsCursorVisible()) {
			Log("CURSOR ON");
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else {
			Log("CURSOR OFF");
			SetCursor(NULL);
		}
		return 0;
	}
	break;
	case WM_CLOSE:
	{
		pn::menu_handle hMenu;
		hMenu = GetMenu(hwnd);
		if (hMenu != NULL) {
			DestroyMenu(hMenu);
		}
		DestroyWindow(hwnd);
		pn::app::Exit();
		return 0;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CREATE:
		break;

	case WM_KEYDOWN:
		if (wParam < 256) {
			bool was_down_last_frame = lParam >> 30;
			key_state new_key_state = key_state::PRESSED;
			if (!was_down_last_frame) {
				new_key_state = key_state::JUST_PRESSED;
			}
			pn::input::SetKeyState(static_cast<unsigned int>(wParam), new_key_state);
		}
		break;

	case WM_KEYUP:
		if (wParam < 256) {
			pn::input::SetKeyState(static_cast<unsigned int>(wParam), key_state::JUST_RELEASED);
		}
		break;

	case WM_LBUTTONDOWN:
		pn::input::SetKeyState(VK_LBUTTON, key_state::JUST_PRESSED);
		break;

	case WM_LBUTTONUP:
		pn::input::SetKeyState(VK_LBUTTON, key_state::JUST_RELEASED);
		break;

	case WM_RBUTTONDOWN:
		pn::input::SetKeyState(VK_RBUTTON, key_state::JUST_PRESSED);
		break;

	case WM_RBUTTONUP:
		pn::input::SetKeyState(VK_RBUTTON, key_state::JUST_RELEASED);
		break;

	case WM_MBUTTONDOWN:
		pn::input::SetKeyState(VK_MBUTTON, key_state::JUST_PRESSED);
		break;

	case WM_MBUTTONUP:
		pn::input::SetKeyState(VK_MBUTTON, key_state::JUST_RELEASED);
		break;
		
	case WM_MOUSEWHEEL:
	{
		auto new_state = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? mouse_wheel_state::SCROLL_UP : mouse_wheel_state::SCROLL_DOWN;
		pn::input::SetMouseWheelState(new_state);
	}
		break;

	case WM_MOUSEMOVE:
	{
		auto new_x_c = (signed short) (lParam);
		auto new_y_c = (signed short) (lParam >> 16);
		POINT pt = { new_x_c, new_y_c };
		ClientToScreen(hwnd, &pt);
		signed short new_x = pt.x;
		signed short new_y = pt.y;
		auto current_mouse_pos = input::GetMousePos();
		if (current_mouse_pos.x != new_x || current_mouse_pos.y != new_y) {
			if (input::IsCursorLocked()) {
				input::SetMouseDelta(new_x - static_cast<mouse_coord_t>(app::window_desc.width / 2), new_y - static_cast<mouse_coord_t>(app::window_desc.height / 2));
				input::MoveCursorToCenter();
			}
			else {
				input::SetMousePos({ new_x, new_y });
				input::SetMouseDelta(new_x - current_mouse_pos.x, new_y - current_mouse_pos.y);
			}
		}
	}
		break;
	
	case WM_CHAR:
		pn::input::AddInputCharacter(static_cast<unsigned char>(wParam));
		break;
	
	case WM_QUIT:
		pn::app::Exit();
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

	// Load Custom Configuration
	pn::app::LoadEngineConfiguration();

	// INIT WINDOWS

	auto& awd		= pn::app::window_desc;
	awd.h_instance	= hInstance;
	h_wnd			= pn::CreateApplicationWindow(awd, WindowProc);

	// INIT DIRECTX
	auto device	= pn::CreateDevice();
	pn::SetDevice(device);

	// ------- POST DX INIT ---------
	InitRenderSystem(h_wnd, awd);
	InitMeshLoadUtil(device);
	debug::InitDebugDraw();

	// ------ SET UP IMGUI ------------------------------
	
	{
		auto context = pn::GetContext(device);
		ImGui_ImplDX11_Init(h_wnd, device.Get(), context.Get());
		pn::gui::InitEditorUI();
	}

	bool display_imgui		= true;
	bool show_test_window	= false;
	bool show_command_line	= false;
	bool show_main_menu		= true;

	// ------- USER-DEFINED INIT CALL -----------------
	Init();

	// ----- DISPLAY WINDOW -----------
	ShowWindow(h_wnd, nCmdShow);

	// ----- MAIN LOOP------------

	bool bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	auto prev_time        = std::chrono::system_clock::now();
	double time_to_process = 0;
	pn::app::FPS          = 60.0f;
	pn::app::FIXED_DT     = 1 / pn::app::FPS;
	while (!pn::app::ShouldExit()) {

		// BEGINNING OF FRAME CALLS
		MainLoopBegin();

		InputUpdate();

		// GET INPUT
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		// Draw main menu
		pn::gui::SetMainMenuVisible(show_main_menu);

		// Draw command line
		show_command_line = pn::input::GetKeyState(input_key::TILDE) == key_state::JUST_PRESSED ? !show_command_line : show_command_line;
		pn::gui::SetCommandLine(show_command_line);

		// Update render system
		// Render system is updated before Update functions are called
		// so debug draw calls work properly
		ClearDepthStencilView(DISPLAY_DEPTH_STENCIL, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		ClearRenderTargetView(DISPLAY_RENDER_TARGET, vec4f(0, 0, 0, 1));
		SetRenderTarget(DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);

		// UPDATE
		auto current_time = std::chrono::system_clock::now();
		
		app::dt = std::chrono::duration<double>(current_time - prev_time).count();
		time_to_process   += pn::app::dt;
		app::running_time += pn::app::dt;
		prev_time         = current_time;

		while (time_to_process >= pn::app::FIXED_DT) {
			FixedUpdate();
			time_to_process -= pn::app::FIXED_DT;
		}

		Update();

		// RENDER

		UpdateGlobalConstantCBuffer();
		UpdateCameraConstantCBuffer(MAIN_CAMERA);

		// Start new frame
		ImGui_ImplDX11_NewFrame();

		if (show_test_window) {
			ImGui::ShowTestWindow(nullptr);
		}

		// Draw main menu
		pn::gui::DrawMainMenu(awd.width);

		// Draw command line
		pn::gui::DrawCommandLine();

		// USER-DEFINED RENDER CALL
		Render();

		ImGui::Render();

		auto hr = SWAP_CHAIN->Present(1, 0);
		if (FAILED(hr)) {
			LogError("Swap chain present error: ", pn::ErrMsg(hr));
		}

		// END OF FRAME CALLS
		pn::input::InputOnEndOfFrame();
		MainLoopEnd();
	}

	// Shutdown
	pn::gui::ShutdownEditorUI();
	pn::CloseLogger();

#ifndef NDEBUG	
	ID3D11Debug* DebugDevice = nullptr;
	HRESULT Result = device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&DebugDevice));

	Result = DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	DebugDevice->Release();
#endif

	Close();
}
