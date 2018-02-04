#include "GlobalConstants.hlsli"

TextureCube cubemap : register(t1);

SamplerState cube_sampler : register(s1) {
	Filter   = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

// ------- INPUT / OUTPUT ---

struct VS_IN {
	float3 pos	: POSITION;
};

struct VS_OUT {
	float4 screen_pos	  : SV_POSITION;
	float3 cubemap_lookup : POSITION;
};

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;

	float4 pos     = float4(i.pos.xyz, 1.0);
	o.screen_pos   = mul(MVP, pos);
	o.screen_pos.z = o.screen_pos.w;

	o.cubemap_lookup = i.pos;

	return o;
}

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 c = cubemap.Sample(cube_sampler, i.cubemap_lookup).xyz;
	return float4(c, 1);
}