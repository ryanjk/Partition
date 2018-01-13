#include <Application\MainLoop.inc>

void Init() {
	pn::SetWorkingDirectory("C:/Users/Ryan/Documents/Visual Studio 2017/Projects/Partition/");
	pn::SetResourceDirectoryName("Resources");
}

void Update(const float dt) {}

void Render() {
	auto context = pn::GetContext(device);

	// Update global uniforms
	global_constants.data.t += static_cast<float>(pn::app::dt);
	auto screen_desc = pn::GetTextureDesc(pn::GetSwapChainBackBuffer(swap_chain));
	global_constants.data.screen_width = static_cast<float>(screen_desc.Width);
	global_constants.data.screen_height = static_cast<float>(screen_desc.Height);

	// Set render target backbuffer color
	float color[] = { 0.0f, 0.0f, 0.0f, 1.000f };
	context->ClearRenderTargetView(display_render_target.Get(), color);
	context->ClearDepthStencilView(display_depth_stencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, display_render_target.GetAddressOf(), display_depth_stencil.Get());

}

void MainLoopBegin() {}

void MainLoopEnd() {}