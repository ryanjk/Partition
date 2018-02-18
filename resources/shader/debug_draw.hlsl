#include "GlobalConstants.hlsli"

cbuffer options : register(b4) {
	float4 color;
}

struct VS_IN {
	float3 pos	: POSITION;
};

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
};

VS_OUT VS_main(VS_IN i) {
	float4 pos   = float4(i.pos.xyz, 1.0);

	VS_OUT o;
	o.screen_pos = mul(PROJECTION, mul(VIEW, pos));
	return o;
}

float4 PS_main(VS_OUT i) : SV_TARGET{
	return color;
}