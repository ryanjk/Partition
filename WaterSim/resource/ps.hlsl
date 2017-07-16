#include "GlobalConstants.hlsli"

Texture2D		tex	: register(t0);
SamplerState	ss	: register(s0);

cbuffer InstanceConstants : register(b1) {
	float4x4 model;
	float4x4 view;
	float4x4 proj;
}

cbuffer DirectionalLight : register(b2) {
	float3	direction;
	float	intensity;
}

struct PS_IN {
	float4 screen_pos	: SV_POSITION;
	float4 world_pos	: POSITION;
	float4 n			: NORMAL;
	float2 uv			: TEXCOORD0;
};

float4 main(PS_IN i) : SV_TARGET
{
	float3 shade = max(0.0, dot(i.n, -direction)) * intensity;
	float4 view_pos = float4(view[3][0], view[3][1], view[3][2], 1);
	float4 spec = pow(max(0.0, dot(normalize(view_pos - i.world_pos), i.n)), 10000);
	float4 tex_color = float4(i.uv, 0, 1);
	return float4(shade*tex_color.rgb + intensity*spec.xyz, 1.0);
}