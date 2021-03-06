#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

Texture2D display_texture : register(t1);
SamplerState ss			  : register(s1);

// ----- INPUT / OUTPUT --------

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
	float2 uv		  : TEXCOORD0;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN_SCREEN i) {
	VS_OUT o;
	o.screen_pos = float4(i.pos, 1);
	o.uv = (float2(1, 1) + i.pos.xy) * 0.5;
	o.uv.y = 1 - o.uv.y;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 tex = display_texture.Sample(ss, i.uv).xyz;
	return float4(tex, 1);
}