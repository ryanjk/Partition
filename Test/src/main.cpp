#include <Graphics\Window.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	/*if (ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
		return true;
	} */

	switch (uMsg) {
	case WM_SIZE:
		/*if (device.Get() != nullptr && wParam != SIZE_MINIMIZED) {
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
		}*/
		return 0;
	case WM_CLOSE:
	{
		HMENU hMenu;
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	pn::CreateConsole();

	if (hInstance == NULL) {
		hInstance = (HINSTANCE) GetModuleHandle(NULL);
	}

	pn::application_window_desc awd;
	awd.h_instance = hInstance;
	awd.width = 1366;
	awd.height = 768;
	awd.fullscreen = false;
	auto h_wnd = pn::CreateApplicationWindow(awd, WindowProc);
	
	//ShowWindow(h_wnd, nCmdShow);
	while (true);
}