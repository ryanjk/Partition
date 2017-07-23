#include "GlobalConstants.hlsli"

#define N_WAVES 1
#define USE_GERSTNER

cbuffer camera_constants : register(b1) {
	float4x4 view;
	float4x4 proj;
}

cbuffer instance_constants : register(b2) {
	float4x4 model;
}

struct Wave {
	float	A; // amplitude
	float	L; // speed
	float	w; // frequency
	float	q; // factor for Gerstener waves
	float2	d; // direction
};

cbuffer wave : register(b3) {
	Wave w[N_WAVES];
}

struct VS_IN {
	float3 n	: NORMAL;
	float3 t	: TANGENT;
	float3 b	: TANGENT1;
	float3 pos	: POSITION;
	float2 uv	: TEXCOORD0;
};

struct VS_OUT {
	float4 screen_pos	: SV_POSITION;
	float4 world_pos	: POSITION;
	float4 n			: NORMAL;
	float2 uv			: TEXCOORD0;
};

float wave_f(Wave w, float2 p, float t) {
	return dot(w.d, p)*w.w + t*(w.L*w.w);
}
float sin_wave_f(Wave w, float2 p, float t) {
	return sin(wave_f(w,p,t));
}
float cos_wave_f(Wave w, float2 p, float t) {
	return cos(wave_f(w,p,t));
}

#ifndef USE_GERSTNER

float sample_wave_x(Wave w, float2 p, float t) {
	return 0;
}
float sample_wave_y(Wave w, float2 p, float t) {
	return 0;
}
float sample_wave_height(Wave w, float2 p, float t) {
	return w.A * sin_wave_f(w,p,t);
}

float sample_wave_nx(Wave w, float2 p, float t) {
	return w.A * w.d.x * p.x * w.w * cos_wave_f(w,p,t);
}
float sample_wave_ny(Wave w, float2 p, float t) {
	return w.A * w.d.y * p.y * w.w * cos_wave_f(w,p,t);
}
float sample_wave_nz(Wave w, float2 p, float t) {
	return 0;
}

#else

float sample_wave_x(Wave w, float2 p, float t) {
	return w.A * w.q * w.d.x * cos_wave_f(w,p,t);
}
float sample_wave_y(Wave w, float2 p, float t) {
	return w.A * w.q * w.d.y * cos_wave_f(w, p, t);
}
float sample_wave_height(Wave w, float2 p, float t) {
	return w.A * sin_wave_f(w, p, t);
}

float sample_wave_nx(Wave w, float2 p, float t) {
	return w.A * w.d.x * w.w * cos_wave_f(w, p, t);
}
float sample_wave_ny(Wave w, float2 p, float t) {
	return w.A * w.d.y * w.w * cos_wave_f(w, p, t);
}
float sample_wave_nz(Wave w, float2 p, float t) {
	return w.q * w.w * w.A * sin_wave_f(w, p, t);
}

#endif

VS_OUT main(VS_IN i) {
	VS_OUT o;

	const float2 p	= i.pos.xy;

	float x			= p.x;
	float y			= p.y;
	float height	= 0;

	float nx		= 0;
	float ny		= 0;
	float nz		= 1;
	
	for (int w_i = 0; w_i < N_WAVES; w_i++) {
		Wave wv = w[w_i];

		x		+= sample_wave_x(wv, p, TIME);
		y		+= sample_wave_y(wv, p, TIME);
		height	+= sample_wave_height(wv, p, TIME);

		nx		-= sample_wave_nx(wv, p, TIME);
		ny		-= sample_wave_ny(wv, p, TIME);
		nz		-= sample_wave_nz(wv, p, TIME);
	}

	float4 pos		= float4(x, y, -height, 1.0);
	pos				= mul(model, pos);
	o.world_pos		= pos;
	pos				= mul(view, pos);
	o.screen_pos	= mul(proj, pos);

	float3 normal = normalize(float3(nx, ny, nz));
	float3x3 btn = {
		i.b,
		i.t,
		i.n
	};
	o.n = float4(normalize(mul(btn, normal)),0);
	o.n = mul(model, o.n);
	o.n = mul(view, o.n);

	o.uv = i.uv;
	return o;
}