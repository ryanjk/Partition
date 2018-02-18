#include <Graphics\Window.h>
#include <Graphics\DirectX.h>
#include <Graphics\MeshLoadUtil.h>
#include <Graphics\TextureLoadUtil.h>
#include <Graphics\ProjectionMatrix.h>
#include <Graphics\RenderSystem.h>
#include <Graphics\GBuffer.h>

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

#include <System\Flycam.h>

using namespace pn;


// --------------
// --- STATES ---
// --------------

dx_depth_stencil_state less_equal_depth;
dx_blend_state         additive_blend;

// -------------------
// --- RENDER DATA ---
// -------------------

// == Deflate shader ==
shader_program_t deflate_shader;


cbuffer<deferred_material_t> material;

transform_t   plane_transform;
mesh_buffer_t plane_mesh;

transform_t   cubemap_transform;
mesh_buffer_t cubemap_mesh;

void Init() {
	LoadMesh(GetResourcePath("cubemap.fbx"));
	LoadMesh(GetResourcePath("plane.fbx"));
	cubemap_mesh = rdb::GetMeshResource("Cubemap");
	plane_mesh   = rdb::GetMeshResource("Plane");
	plane_transform.position = { 0, 0, 3 };

	InitGBuffers();

	InitializeCBuffer(material);
	material.data.albedoMetal = vec4f(0.5, 0.5, 0.5, 0.5);
	material.data.specRoughness = vec4f(0.5, 0.5, 0.5, 0.5);

	CD3D11_BLEND_DESC blend_desc(D3D11_DEFAULT);
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	additive_blend = CreateBlendState(&blend_desc);

	CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	less_equal_depth = CreateDepthStencilState(&desc);
}

void Resize() {
	ResizeGBuffers();
}

void Update() {
	static bool fon = false;
	if (input::GetKeyState(SPACE) == input::key_state::JUST_PRESSED) {
		fon = !fon;
		input::SetCursorVisible(!input::IsCursorVisible());
		input::SetCursorLock(!input::IsCursorLocked());
	}

	if (fon) {
		UpdateFlycam(MAIN_CAMERA.transform, 10.0f, 0.1f);
	}
}

void FixedUpdate() {}

void Render() {
	ClearGBufferRenderTargets();


}

void MainLoopBegin() {}

void MainLoopEnd() {}

void Close() {}