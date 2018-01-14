#include "GlobalConstants.hlsli"

Texture2D offscreen_texture : register(t1);

SamplerState ss			: register(s1);

// ----- INPUT / OUTPUT --------

struct VS_IN {
	float3 pos : POSITION;
};

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
	float2 uv		  : TEXCOORD0;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	o.screen_pos = float4(i.pos.x, i.pos.y, 0, 1);
	o.uv         = float2(i.pos.x, i.pos.y) * 0.5 + 0.5;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{
	i.uv.y = 1 - i.uv.y;
	float3 tex = offscreen_texture.Sample(ss, i.uv).xyz;
	return float4(tex.r, 0, tex.g, 1);
}