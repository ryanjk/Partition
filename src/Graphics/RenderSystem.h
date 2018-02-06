#pragma once

#include <Graphics\CBuffer.h>
#include <Graphics\DirectX.h>

#include <Component\transform_t.h>

namespace pn {

// ---- CLASS -----

struct alignas(16) global_constants_t {
	float t = 0.0f;
	float screen_width;
	float screen_height;
};

struct camera_constants_t {
	pn::mat4f view;
	pn::mat4f proj;
};

struct model_constants_t {
	pn::mat4f model;
	pn::mat4f model_view_inverse_transpose;
	pn::mat4f mvp;
};

struct camera_t {
	pn::ProjectionMatrix	projection_matrix;
	pn::transform_t			transform;
};

// ----- GLOBALS ------

extern pn::dx_swap_chain SWAP_CHAIN;

extern pn::cbuffer<global_constants_t>	global_constants;
extern pn::cbuffer<camera_constants_t>	camera_constants;
extern pn::cbuffer<model_constants_t>	model_constants;

extern pn::dx_render_target_view    DISPLAY_RENDER_TARGET;
extern pn::dx_depth_stencil_view    DISPLAY_DEPTH_STENCIL;
extern pn::camera_t					MAIN_CAMERA;

extern shader_program_t* CURRENT_SHADER;

// ----- FUNCTIONS -----

void InitRenderSystem(const window_handle hwnd, const application_window_desc awd);

void UpdateGlobalConstantCBuffer();
void UpdateCameraConstantCBuffer(const camera_t& camera);
void UpdateModelConstantCBuffer(const transform_t& transform);

// ----- STATE ACCESS -------

void ResetRenderTarget();

void ClearDepthStencil();
void ClearDisplay(const pn::vec4f color = { 0.0f, 0.0f, 0.0f, 1.000f });

void SetShaderProgram(shader_program_t& shader_program);
void SetStandardShaderProgram(shader_program_t& shader_program);
void ClearShaderProgram();

void SetVertexBuffers(const mesh_buffer_t& mesh_buffer);
void ClearVertexBuffers();

void SetProgramConstant(const pn::string& buffer_name, const dx_buffer& buffer);
void SetProgramResource(const pn::string& resource_name, dx_resource_view& resource_view);
void SetProgramSampler(const pn::string& sampler_name, dx_sampler_state& sampler_state);

void SetAlphaBlend(bool on, int num_render_targets = 1);
void SetDepthTest(bool on);
void SetWireframeMode(bool on);

}