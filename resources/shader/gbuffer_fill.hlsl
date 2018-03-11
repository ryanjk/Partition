#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

SamplerState tex_sampler : register(s1) {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

#ifdef USE_MATERIAL_TEX
Texture2D albedo   : register(t1);
Texture2D specular : register(t2);
#else
cbuffer material {
	float4 albedo;
	float4 specular;
}
#endif

#define USE_HEIGHT_MAP

Texture2D height_map : register(t3);
cbuffer height_map_params {
	float height_map_scale;
}


// ------- INPUT / OUTPUT ---

struct VS_OUT {
	float4 screen_pos	: SV_POSITION;
	float4 world_pos	: POSITION;
	float4 n			: NORMAL;
	float4 t            : TANGENT0;
	float4 b            : TANGENT1;
	float2 uv			: TEXCOORD0;
};

// ----- VERTEX SHADER -----

VS_OUT VS_main(VS_IN_FULL i) {
	VS_OUT o;

	float4 pos   = float4(i.pos, 1.0);
	o.world_pos  = mul(MODEL, pos);

	float4 n_w = float4(i.n, 0.0);
	n_w = mul(MODEL, n_w);
	n_w = normalize(n_w);

	o.world_pos += float4(height_map.SampleLevel(tex_sampler, i.uv, 0).x * n_w.xyz * height_map_scale, 0);

	o.n = normalize(float4(i.n, 0));
	o.t = normalize(float4(i.t,0));
	o.b = normalize(float4(i.b,0));

	o.screen_pos = mul(PROJECTION, mul(VIEW, o.world_pos));
	o.uv = i.uv;
	return o;
}

// ------- PIXEL SHADER ---------

struct PS_OUT {
	float4 albedo   : SV_TARGET0;
	float world     : SV_TARGET1;
	float4 normal   : SV_TARGET2;
	float4 specular : SV_TARGET3;
};

PS_OUT PS_main(VS_OUT i) {
	PS_OUT o;

#ifdef USE_MATERIAL_TEX
	o.albedo   = albedo.Sample(tex_sampler, i.uv);
	o.specular = specular.Sample(tex_sampler, i.uv);
#else
	o.albedo   = albedo;
	o.specular = specular;
#endif

#ifdef USE_HEIGHT_MAP
	const float DELTA = 0.01;
	float h_mx = height_map.SampleLevel(tex_sampler, i.uv - float2(DELTA, 0), 0).x * height_map_scale;
	float h_px = height_map.SampleLevel(tex_sampler, i.uv + float2(DELTA, 0), 0).x * height_map_scale;
	float h_my = height_map.SampleLevel(tex_sampler, i.uv - float2(0, DELTA), 0).x * height_map_scale;
	float h_py = height_map.SampleLevel(tex_sampler, i.uv + float2(0, DELTA), 0).x * height_map_scale;

	float3 va = normalize(float3(2 * DELTA, 0, h_mx - h_px));
	float3 vb = normalize(float3(0, 2 * DELTA, h_py - h_my));
	float3 n = normalize(cross(va, vb));

	float3x3 tbn = { i.t.xyz, i.b.xyz, i.n.xyz };
	float3 n_p = mul(MODEL, mul(tbn, n));
	o.normal = float4(n_p,0);
#else
	o.normal = float4(normalize(i.n.xyz),0);
#endif

	o.world = i.screen_pos.z;

	return o;
}