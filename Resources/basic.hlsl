#include "GlobalConstants.hlsli"

// ---- CONSTANT BUFFERS --------

cbuffer directional_light : register(b4) {
	float3	direction;
	float	intensity;
}

// ----- INPUT / OUTPUT --------

struct VS_IN {
	float3 n : NORMAL;
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
	float4 world_pos : POSITION;
	float2 uv : TEXCOORD0;
	float4 n : TEXCOORD1;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	float4 pos = float4(i.pos, 1.0);
	pos = mul(MODEL, pos);
	o.world_pos = pos;
	pos = mul(VIEW, pos);
	o.screen_pos = mul(PROJECTION, pos);

	o.n = float4(i.n, 0.0);
	o.n = mul(MODEL, o.n);
	//o.n = mul(VIEW, o.n);
	o.n = normalize(o.n);

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{
	float ndotl			= max(0.0, dot(i.n.xyz, -direction));
	float3 shade		= ndotl * intensity;

	float4 view_pos		= float4(VIEW[3][0], VIEW[3][1], VIEW[3][2], 1);
	float4 view_dir		= normalize(view_pos - i.world_pos);
	float3 reflected	= 2 * ndotl * i.n.xyz + direction;
	float view_angle	= max(0.0, dot(view_dir, float4(reflected, 0)));
	float4 spec			= pow(view_angle, 100);

	float4 color		= float4(1, 0, 0, 1);
	return float4(shade*(color.rgb + spec.rgb), 1);
}