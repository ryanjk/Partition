#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

#ifdef USE_MATERIAL_TEX
Texture2D albedo   : register(t1);
Texture2D specular : register(t2);
#else
cbuffer material {
	float4 albedo;
	float4 specular;
}
#endif

SamplerState tex_sampler : register(s1) {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

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

	float4 pos   = float4(i.pos, 1.0);
	o.world_pos  = mul(MVP, pos);
	o.screen_pos = mul(MVP, pos);

	o.n = float4(i.n, 0.0);
	o.n = mul(MODEL, o.n);
	o.n = normalize(o.n);

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