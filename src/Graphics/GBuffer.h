#pragma once

#include <Graphics\DirectX.h>

#include <UI\EditorUI.h>
#include <UI\EditStruct.h>

namespace pn {

// -------------------------
// --- CLASS DEFINITIONS ---
// -------------------------

struct gbuffer_t {
	pn::dx_render_target_view render_target;
	pn::dx_resource_view      texture;
};

struct alignas(16) deferred_material_t {
	vec4f albedoMetal;
	vec4f specRoughness;
};

template<>
void gui::EditStruct(deferred_material_t& material);

// -------------
// --- STATE ---
// -------------

extern gbuffer_t ALBEDO_GBUFFER;
extern gbuffer_t DEPTH_GBUFFER;
extern gbuffer_t NORMAL_GBUFFER;
extern gbuffer_t SPECULAR_GBUFFER;

extern shader_program_t GBUFFER_FILL;
extern shader_program_t DEFERRED_LIGHTING;
extern shader_program_t DEFERRED_CUBEMAP_LIGHTING;

// -----------------
// --- FUNCTIONS ---
// -----------------

void InitGBuffers();
void InitGBuffer(gbuffer_t& g, CD3D11_TEXTURE2D_DESC d);

void ResizeGBuffers();
void ResizeGBuffer(gbuffer_t& g, int width, int height);

void ClearGBufferRenderTargets();
void SetGBufferRenderTargets();
void SetDeferredShaderProgram(shader_program_t& shader);

}


