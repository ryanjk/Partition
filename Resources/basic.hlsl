#include "GlobalConstants.hlsli"

// ---- CONSTANT BUFFERS --------

cbuffer camera_constants {
	float4x4 view;
	float4x4 proj;
}

cbuffer model_constants {
	float4x4 model;
	float4x4 mvp;
}

cbuffer directional_light {
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
	float4 n : NORMAL;
	float2 uv : TEXCOORD0;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	float4 pos = float4(i.pos, 1.0);
	pos = mul(model, pos);
	o.world_pos = pos;
	pos = mul(view, pos);
	o.screen_pos = mul(proj, pos);

	o.n = float4(i.n, 0.0);
	o.n = mul(model, o.n);
	o.n = mul(view, o.n);

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 shade = max(0.0, dot(i.n.xyz, -direction)) * intensity;
	float4 view_pos = float4(view[3][0], view[3][1], view[3][2], 1);

	float view_angle = max(0.0, dot(normalize(view_pos - i.world_pos), i.n));
	float4 spec = pow(view_angle, 10000);
	float4 color = float4(1, 0, 0, 1);

	return float4(shade*(color.rgb + spec.rgb), 1);
}