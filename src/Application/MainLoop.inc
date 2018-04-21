#pragma once

#include <Graphics\Window.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <d3d12sdklayers.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <wrl.h>

/*
#include <Graphics\DirectX.h>
#include <Graphics\RenderSystem.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\CBuffer.h>
#include <Graphics\DebugDraw.h>
*/

#include <Graphics\MeshLoadUtil.h>

#include <Utilities\Logging.h>
#include <Utilities\Memory.h>

#include <Input\Input.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

//#include <UI\UIUtil.h>
//#include <UI\EditorUI.h>

#include <Application\Global.h>

#include <chrono>

using namespace pn;
using namespace pn::input;

using Microsoft::WRL::ComPtr;

// ======================
// == HELPER FUNCTIONS ==
// ======================

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
}

void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) {
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			char description[256];
			
			Log("Device information:");
			
			wcstombs(description, desc.Description, 256);
			Log("Graphics Card: {}", description);
			Log("Dedicated video memory: {} B", desc.DedicatedVideoMemory);
			Log("Dedicated system memory: {} B", desc.DedicatedSystemMemory);
			Log("Shared system memory: {} B", desc.SharedSystemMemory);
			
			break;
		}
	}
	*ppAdapter = adapter.Detach();
}

// =====================
// === DIRECTX STATE ===
// =====================

ComPtr<ID3D12PipelineState> pipeline_state;

ComPtr<ID3D12Device>			device;
ComPtr<ID3D12CommandQueue>		command_queue;
ComPtr<ID3D12GraphicsCommandList> command_list;
ComPtr<ID3D12CommandAllocator>	command_allocator;
ComPtr<IDXGISwapChain3>			swap_chain;

ComPtr<ID3D12DescriptorHeap>	rtv_heap;
const unsigned int				NUM_FRAMES = 2;
ComPtr<ID3D12Resource>			render_targets[NUM_FRAMES];

ComPtr<ID3D12Fence> fence;
HANDLE			fence_event;
UINT64			fence_value;
unsigned int	frame_index;

ComPtr<ID3D12RootSignature> root_signature;

CD3DX12_VIEWPORT viewport;
CD3DX12_RECT scissor_rect;

ComPtr<ID3D12Resource>		vertex_buffer;
D3D12_VERTEX_BUFFER_VIEW	vertex_buffer_view;

unsigned int rtv_descriptor_size;

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
//extern IMGUI_API LRESULT	ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

pn::window_long CALLBACK	WindowProc(pn::window_handle hwnd, unsigned int uMsg, pn::window_uint wParam, pn::window_long lParam) {
	/*if (pn::gui::IsGUIOn()) {
		ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam);
	}	*/

	switch (uMsg) {
	case WM_SIZE:
		/*if (GetDevice() != nullptr && wParam != SIZE_MINIMIZED) {
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
		}*/
		return 0;
	case WM_SETCURSOR:
	{

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

	// === CREATE DEBUG LAYER ===
	unsigned int dxgi_factory_flags = 0;
#ifndef NDEBUG
	ComPtr<ID3D12Debug> dbg_controller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbg_controller)))) {
		dbg_controller->EnableDebugLayer();
		dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	
	// === CREATE DIRECTX DEVICE ===
	ComPtr<IDXGIFactory4> dxgi_factory;
	CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory));
	ComPtr<IDXGIAdapter1> hardware_adapter;
	GetHardwareAdapter(dxgi_factory.Get(), &hardware_adapter);
	D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	
	// === CREATE COMMAND QUEUE ===
	D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
	command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	command_queue_desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&command_queue)));

	// === CREATE SWAP CHAIN ===
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.BufferCount = NUM_FRAMES;
	swap_chain_desc.Width		= pn::app::window_desc.width;
	swap_chain_desc.Height		= pn::app::window_desc.height;
	swap_chain_desc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect	= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.SampleDesc.Count = 1;
	ComPtr<IDXGISwapChain1> tmp_swap_chain;
	ThrowIfFailed(dxgi_factory->CreateSwapChainForHwnd(command_queue.Get(), h_wnd, &swap_chain_desc, nullptr, nullptr, &tmp_swap_chain));
	tmp_swap_chain.As(&swap_chain);

	frame_index = swap_chain->GetCurrentBackBufferIndex();
	// === CREATE DESCRIPTOR HEAP FOR RENDER TARGET VIEWS ===
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
	rtv_heap_desc.NumDescriptors = NUM_FRAMES;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));
	rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// === CREATE RENDER TARGET VIEWS ===
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());
	for (unsigned int n = 0; n < 2; ++n) {
		ThrowIfFailed(swap_chain->GetBuffer(n, IID_PPV_ARGS(&render_targets[n])));
		device->CreateRenderTargetView(render_targets[n].Get(), nullptr, rtv_handle);
		rtv_handle.Offset(1, rtv_descriptor_size);
	}

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));

	// ------- POST DX INIT ---------

	// === CREATE ROOT SIGNATURE ===
	CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
	root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)));

	// === CREATE SHADERS ===
	ComPtr<ID3DBlob> vertex_shader;
	ComPtr<ID3DBlob> pixel_shader;
#ifndef NDEBUG
	UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flags = 0;
#endif
	const pn::string shader_path = GetResourcePath("dx12_basic.hlsl");
	pn::wstring shader_file(shader_path.size(), L'#');
	mbstowcs(shader_file.data(), shader_path.c_str(), shader_path.size());
	ThrowIfFailed(D3DCompileFromFile(shader_file.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertex_shader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(shader_file.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &pixel_shader, nullptr));

	D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// === CREATE PIPELINE STATE ===
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.InputLayout		= { input_element_descs, _countof(input_element_descs) };
	pso_desc.pRootSignature		= root_signature.Get();
	pso_desc.VS					= CD3DX12_SHADER_BYTECODE(vertex_shader.Get());
	pso_desc.PS					= CD3DX12_SHADER_BYTECODE(pixel_shader.Get());
	pso_desc.RasterizerState	= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.BlendState			= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState.DepthEnable = FALSE;
	pso_desc.DepthStencilState.StencilEnable = FALSE;
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.SampleDesc.Count = 1;
	ThrowIfFailed(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));

	// === CREATE VIEWPORT AND SCISSOR RECT ===
	viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float) pn::app::window_desc.width, (float) pn::app::window_desc.height);
	scissor_rect = CD3DX12_RECT(0, 0, (long) pn::app::window_desc.width, (long) pn::app::window_desc.height);

	// === CREATE GRAPHICS COMMAND LIST ===
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)));
	ThrowIfFailed(command_list->Close());

	// === CREATE VERTEX BUFFER ===
	struct Vertex {
		vec3f position;
		vec4f color;
	};
	const float aspect_ratio = pn::app::window_desc.width / (float)pn::app::window_desc.height;
	Vertex vertices[] = {
		{ { 0.0f, 0.25f * aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f * aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f * aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f} }
	};
	const UINT vertex_buffer_size = sizeof(vertices);
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertex_buffer)
	));

	UINT8* vertex_data_begin;
	CD3DX12_RANGE read_range(0, 0);
	ThrowIfFailed(vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin)));
	memcpy(vertex_data_begin, vertices, vertex_buffer_size);

	vertex_buffer_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
	vertex_buffer_view.StrideInBytes = sizeof(Vertex);
	vertex_buffer_view.SizeInBytes = vertex_buffer_size;

	// === CREATE SYNCHRONIZATION OBJECTS ===
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fence_value = 1;
	fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fence_event == nullptr) {
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// ------ SET UP IMGUI ------------------------------

	//pn::gui::SetGUI(true);
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

		// GET INPUT
		bGotMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
		if (bGotMsg) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		
		// BEGINNING OF FRAME CALLS
		MainLoopBegin();

		// Draw main menu
		//pn::gui::SetMainMenuVisible(show_main_menu);

		// Draw command line
		show_command_line = pn::input::GetKeyState(input_key::TILDE) == key_state::JUST_PRESSED ? !show_command_line : show_command_line;
		//pn::gui::SetCommandLine(show_command_line);

		// Toggle imGUI
		if (pn::input::GetKeyState(input_key::I) == key_state::JUST_PRESSED) {
			//pn::gui::SetGUI(!pn::gui::IsGUIOn());
		}

		//if (pn::gui::IsGUIOn()) {
			//ImGui_ImplDX11_NewFrame();
		//}

		// Update render system
		// Render system is updated before Update functions are called
		// so debug draw calls work properly

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

		// Start new frame
		/*if (pn::gui::IsGUIOn()) {

			if (show_test_window) {
				ImGui::ShowTestWindow(nullptr);
			}

			// Draw main menu
			pn::gui::DrawMainMenu(awd.width);

			// Draw command line
			pn::gui::DrawCommandLine();
		}*/

		// USER-DEFINED RENDER CALL
		Render();

		/*if (pn::gui::IsGUIOn()) {
			ImGui::Render();
		}*/

		ThrowIfFailed(swap_chain->Present(1, 0));
		if (FAILED(hr)) {
			LogError("Swap chain present error: ", pn::ErrMsg(hr));
		}

		// END OF FRAME CALLS
		pn::input::InputOnEndOfFrame();
		MainLoopEnd();
	}

	// Shutdown
	//pn::gui::ShutdownEditorUI();
	pn::CloseLogger();

#ifndef NDEBUG	

#endif

	Close();
}
