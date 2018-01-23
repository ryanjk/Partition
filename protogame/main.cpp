#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>

#include <Utilities\Logging.h>
#include <Utilities\frame_string.h>
#include <Utilities\Profile.h>

#include <IO\FileUtil.h>
#include <IO\PathUtil.h>

#include <UI\UIUtil.h>
#include <UI\EditorUI.h>
#include <UI\EditStruct.h>

#include <Component\transform_t.h>

#include <chrono>

#include <Application\ResourceDatabase.h>
#include <Application\MainLoop.inc>

void Init() {}

void Update(const double dt) {}

void Render() {
	pn::ClearDepthStencilView(display_depth_stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	static const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.000f };
	pn::ClearRenderTargetView(display_render_target, color);

	pn::SetRenderTarget(display_render_target, display_depth_stencil);
}

void MainLoopBegin() {}

void MainLoopEnd() {}

void Close() {}