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

// ---------------
// --- CBUFFER ---
// ---------------

struct alignas(16) light_t {
	vec3f light_position; float p;
	vec3f light_color;    float p2;
	float light_intensity;
};

template<>
void pn::gui::EditStruct(light_t& light) {
	DragFloat3("position##", &light.light_position.x, -10, 10);
	DragFloat3("color##"   , &light.light_color.x, 0, 1);
	DragFloat("intensity##", &light.light_intensity, 0, 100);
}

#define NUM_LIGHTS 1
light_t lights[NUM_LIGHTS];
cbuffer<light_t> light;

struct alignas(16) environment_lighting_t {
	float environment_intensity;
};

template<>
void pn::gui::EditStruct(environment_lighting_t& light) {
	DragFloat("intensity##", &light.environment_intensity, 0, 100);
}

cbuffer<environment_lighting_t> environment_lighting;

struct alignas(16) height_map_params_t {
	float scale;
};

template<>
void gui::EditStruct(height_map_params_t& h) {
	DragFloat("scale", &h.scale, 0, 5);
}
cbuffer<height_map_params_t> height_map_params;

// --------------
// --- STATES ---
// --------------

dx_depth_stencil_state less_equal_depth;
dx_blend_state         additive_blend;

// -------------------
// --- RENDER DATA ---
// -------------------

dx_sampler_state ss;

cbuffer<deferred_material_t> material;

transform_t   plane_transform;
mesh_buffer_t plane_mesh;

dx_resource_view height_map;

transform_t      cubemap_transform;
mesh_buffer_t    cubemap_mesh;
dx_resource_view cubemap;

void LoadResources() {
	LoadMesh(GetResourcePath("cubemap.fbx"));
	LoadMesh(GetResourcePath("plane.fbx"));

	height_map = LoadTexture2D(GetResourcePath("height.jpg"));
	cubemap = LoadCubemap(GetResourcePath("space-cubemap.dds"));
}

void Init() {


	CD3D11_SAMPLER_DESC sampler_desc(D3D11_DEFAULT);
	ss = pn::CreateSamplerState(sampler_desc);



	cubemap_mesh = rdb::GetMeshResource("Cubemap");
	plane_mesh   = rdb::GetMeshResource("Plane");
	plane_transform.position = { 0, 0, 12.4f };
	plane_transform.rotation = EulerToQuaternion({ 1.06f, 0, 0 });

	InitGBuffers();

	InitializeCBuffer(light);
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		lights[i].light_color = vec3f(1, 1, 1);
		lights[i].light_intensity = 50;
		lights[i].light_position = vec3f(0, 0.5f, 0);
	}

	InitializeCBuffer(environment_lighting);
	environment_lighting.data.environment_intensity = 1.0f;

	InitializeCBuffer(height_map_params);
	height_map_params.data.scale = 1.0f;

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

	ImGui::Begin("Lights");
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		ImGui::PushID(i);
		gui::EditStruct(lights[i]);
		ImGui::PopID();
	}
	ImGui::End();

	ImGui::Begin("Environment");
	gui::EditStruct(environment_lighting.data);
	ImGui::End();
	UpdateBuffer(environment_lighting);

	SetDepthTest(true);
	SetGBufferRenderTargets();
	SetStandardShaderProgram(GBUFFER_FILL);

	ImGui::Begin("Height Map");
	gui::EditStruct(height_map_params.data);
	ImGui::End();
	UpdateBuffer(height_map_params);

	ImGui::Begin("Plane Material");
	gui::EditStruct(material.data);
	ImGui::End();
	UpdateBuffer(material);

	SetProgramSampler("tex_sampler", ss);
	SetProgramConstant("material", material);
	SetProgramConstant("height_map_params", height_map_params);
	SetProgramResource("height_map", height_map);
	SetVertexBuffers(plane_mesh);

	ImGui::Begin("Plane Transform");
	gui::EditStruct(plane_transform);
	ImGui::End();

	UpdateModelConstantCBuffer(plane_transform);
	DrawIndexed(plane_mesh);

	SetRenderTarget(DISPLAY_RENDER_TARGET, nullptr);
	SetDepthTest(false);
	SetBlendState(additive_blend);

	SetDeferredShaderProgram(DEFERRED_LIGHTING);
	SetProgramSampler("ss", ss);
	SetProgramConstant("light", light);
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		light.data = lights[i];
		UpdateBuffer(light);
		_context->Draw(4, 0);
	}

	SetDeferredShaderProgram(DEFERRED_CUBEMAP_LIGHTING);
	SetProgramConstant("environment_lighting", environment_lighting);
	SetProgramResource("environment", cubemap);

	_context->Draw(4, 0);

	SetBlendState();
}

void MainLoopBegin() {}

void MainLoopEnd() {}

void Close() {}