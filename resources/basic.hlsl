#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

// ---- CONSTANT BUFFERS --------

cbuffer directional_light : register(b4) {
	float3	direction;
	float	intensity;
}

// ----- INPUT / OUTPUT --------

struct VS_OUT {
	float4 screen_pos	: SV_POSITION;
	float4 world_pos	: POSITION;
	float2 uv			: TEXCOORD0;
	float4 n			: TEXCOORD1;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN_SIMPLE i) {
	VS_OUT o;
	float4 pos = float4(i.pos, 1.0);
	pos = mul(MODEL, pos);
	o.world_pos = pos;
	pos = mul(VIEW, pos);
	o.screen_pos = mul(PROJECTION, pos);

	o.n = float4(i.n, 0.0);
	o.n = mul(MODEL_VIEW_INVERSE_TRANSPOSE, o.n);
	o.n = normalize(o.n);

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET {
	float3 n			= normalize(i.n.xyz);
	float ndotl			= saturate(dot(n, -direction));
	float3 shade		= ndotl * intensity;

	float3 view_pos		= float3(VIEW[3][0], VIEW[3][1], VIEW[3][2]);
	float3 view_dir		= normalize(view_pos - i.world_pos.xyz);
	float3 halfw		= normalize(view_dir - direction);
	float4 spec			= pow(saturate(dot(n,halfw)), 1000);

	float4 color		= float4(1, 0, 0, 1);
	return float4(shade*(color.rgb + spec.rgb), 1);
}