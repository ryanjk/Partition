#include "GlobalConstants.hlsli"

Texture2D tex	: register(t0);
SamplerState ss : register(s0);

struct PS_IN {
	float4 screen_pos : SV_POSITION;
	float4 world_pos : POSITION;
	float4 n : NORMAL;
	float2 uv : TEXCOORD0;
};

float4 main(PS_IN i) : SV_TARGET
{
	/*float4 light_pos = float4(2.0, 2.0, 0.0, 1.0);
	float4 l = normalize(light_pos - i.world_pos);
	float3 color = max(0.0, dot(i.n, l)) * float3(1.0, 1.0, 1.0);
	return float4(color, 1.0); */
	float4 color = tex.Sample(ss, i.uv);
	return color;
}