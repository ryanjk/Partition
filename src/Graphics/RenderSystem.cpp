#include <Graphics\RenderSystem.h>
#include <Application\Global.h>

namespace pn {

// ----- GLOBALS ------

pn::dx_swap_chain				SWAP_CHAIN;

pn::cbuffer<global_constants_t>	global_constants;
pn::cbuffer<camera_constants_t>	camera_constants;
pn::cbuffer<model_constants_t>	model_constants;

pn::dx_render_target_view		DISPLAY_RENDER_TARGET;
pn::dx_depth_stencil_view		DISPLAY_DEPTH_STENCIL;
pn::camera_t			        MAIN_CAMERA;

// ----- FUNCTIONS -----

void InitRenderSystem(const window_handle h_wnd, const application_window_desc awd) {
	SWAP_CHAIN = pn::CreateMainWindowSwapChain(h_wnd, awd);
	pn::SetViewport(awd.width, awd.height);

	pn::SetRenderTargetAndDepthStencilFromSwapChain(SWAP_CHAIN, DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);

	pn::InitializeCBuffer(model_constants);
	
	pn::InitializeCBuffer(global_constants);
	UpdateGlobalConstantCBuffer();

	pn::InitializeCBuffer(camera_constants);
	MAIN_CAMERA.transform = transform_t{};
	MAIN_CAMERA.projection_matrix = pn::ProjectionMatrix{ pn::ProjectionType::PERSPECTIVE,
		static_cast<float>(pn::app::window_desc.width), static_cast<float>(pn::app::window_desc.height),
		0.01f, 1000.0f,
		70.0f, 0.1f
	};
	UpdateCameraConstantCBuffer(MAIN_CAMERA);
}

void ResetRenderTarget() {
	pn::SetRenderTarget(DISPLAY_RENDER_TARGET, DISPLAY_DEPTH_STENCIL);
}

void UpdateGlobalConstantCBuffer() {
	// Update global uniforms
	global_constants.data.t             += static_cast<float>(pn::app::dt);
	global_constants.data.screen_width  = static_cast<float>(pn::app::window_desc.width);
	global_constants.data.screen_height = static_cast<float>(pn::app::window_desc.height);

	auto screen_desc = pn::GetDesc(pn::GetSwapChainBuffer(SWAP_CHAIN));
	global_constants.data.screen_width = static_cast<float>(screen_desc.Width);
	global_constants.data.screen_height = static_cast<float>(screen_desc.Height);
}

void UpdateCameraConstantCBuffer(const camera_t& camera) {
	// Update camera buffer
	camera_constants.data.view = Inverse(TransformToMatrix(camera.transform));
	camera_constants.data.proj = camera.projection_matrix.GetMatrix();
}

void UpdateModelConstantCBuffer(const transform_t& transform) {
	model_constants.data.model = LocalToWorldMatrix(transform);
	model_constants.data.model_view_inverse_transpose = pn::Transpose(pn::Inverse(model_constants.data.model * camera_constants.data.view));
	model_constants.data.mvp = model_constants.data.model * camera_constants.data.view * camera_constants.data.proj;
}

void ClearDepthStencil() {
	pn::ClearDepthStencilView(DISPLAY_DEPTH_STENCIL, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void ClearDisplay(const pn::vec4f color) {
	pn::ClearRenderTargetView(DISPLAY_RENDER_TARGET, color);
}

}