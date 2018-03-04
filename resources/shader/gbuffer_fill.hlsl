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
	float2 uv			: TEXCOORD0;
};

// ----- VERTEX SHADER -----

VS_OUT VS_main(VS_IN_FULL i) {
	VS_OUT o;

	o.n = float4(i.n, 0.0);
	o.n = mul(MODEL, o.n);
	o.n = normalize(o.n);

	float4 pos   = float4(i.pos, 1.0);
	o.world_pos  = mul(MODEL, pos);

#ifdef USE_HEIGHT_MAP
	o.world_pos += float4(height_map.SampleLevel(tex_sampler, i.uv, 0).x * o.n.xyz * height_map_scale, 0);

	const float DELTA = 0.01;
	float h_mx = height_map.SampleLevel(tex_sampler, i.uv - float2(DELTA, 0), 0).x * height_map_scale;
	float h_px = height_map.SampleLevel(tex_sampler, i.uv + float2(DELTA, 0), 0).x * height_map_scale;
	float h_my = height_map.SampleLevel(tex_sampler, i.uv - float2(0, DELTA), 0).x * height_map_scale;
	float h_py = height_map.SampleLevel(tex_sampler, i.uv + float2(0, DELTA), 0).x * height_map_scale;

	float3 n = normalize(float3((h_mx - h_px) / (2 * DELTA), (h_my - h_py) / (2 * DELTA), -1));
	o.n = normalize(mul(MODEL, float4(n, 0)));
#endif

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

	o.normal = float4(normalize(i.n.xyz),0);
	o.world = i.screen_pos.z;

	return o;
}