#include "GlobalConstants.hlsli"


cbuffer camera_constants : register(b1) {
	float4x4 view;
	float4x4 proj;
}

cbuffer model_constants : register(b2) {
	float4x4 model;
	float4x4 mvp;
}

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


VS_OUT main(VS_IN i) {
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