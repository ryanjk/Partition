#include "GlobalConstants.hlsli"

#define N_WAVES 2

cbuffer InstanceConstants : register(b1) {
	float4x4 model;
	float4x4 view;
	float4x4 proj;
}

struct Wave {
	float A;
	float L;
	float w;
	float padding;
	float2 d;
};

cbuffer WaveBuffer : register(b3) {
	Wave w[N_WAVES];
}

struct VS_IN {
	float3 n : NORMAL;
	float3 t : TANGENT;
	float3 b : TANGENT1;
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
	float4 world_pos : POSITION;
	float4 n : NORMAL;
	float2 uv : TEXCOORD0;
};

float sample_wave(Wave w, float2 p, float t) {
	return w.A * sin(dot(w.d,p)*w.w + t*(w.L*w.w));
}

float sample_wave_dx(Wave w, float2 p, float t) {
	return w.A * w.d.x * p.x * w.w * cos(dot(w.d,p)*w.w + t*(w.L*w.w));
}

float sample_wave_dy(Wave w, float2 p, float t) {
	return w.A * w.d.y * p.y * w.w * cos(dot(w.d,p)*w.w + t*(w.L*w.w));
}

VS_OUT main(VS_IN i) {
	VS_OUT o;

	const float2 p = i.pos.xy;

	float3x3 btn = {
		i.b,
		i.t,
		i.n
	};

	float disp = 0;
	float dx = 0;
	float dy = 0;
	for (int w_i = 0; w_i < N_WAVES; w_i++) {
		Wave wv = w[w_i];
		disp += sample_wave(wv, p, TIME);
		dx += sample_wave_dx(wv, p, TIME);
		dy += sample_wave_dy(wv, p, TIME);
	}

	float3 new_pos = i.pos + i.n.xyz * disp;

	float4 pos = float4(new_pos, 1.0);
	pos = mul(model, pos);
	o.world_pos = pos;
	pos = mul(view, pos);
	o.screen_pos = mul(proj, pos);

	float3 normal = normalize(float3(-dx, -dy, 1));


	o.n = float4(mul(btn, normal),0);
	o.n = mul(model, o.n);
	o.n = mul(view, o.n);

	o.uv = i.uv;
	return o;
}