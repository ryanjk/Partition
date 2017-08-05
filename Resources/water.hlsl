#include "GlobalConstants.hlsli"

// ------ CONSTANT BUFFERS -------

cbuffer directional_light : register(b4) {
	float3	direction;
	float	intensity;
}

struct Wave {
	float	A; // amplitude
	float	L; // speed
	float	w; // wavelength
	float	q; // factor for Gerstener waves
	float2	d; // direction
};

#define N_WAVES 1
cbuffer wave : register(b5) {
	Wave w[N_WAVES];
}

// ------- FUNCTIONS ---------

float wave_f(Wave w, float2 p, float t) {
	return dot(w.d, p)*w.w + t*(w.L*w.w);
}
float sin_wave_f(Wave w, float2 p, float t) {
	return sin(wave_f(w, p, t));
}
float cos_wave_f(Wave w, float2 p, float t) {
	return cos(wave_f(w, p, t));
}

float sample_wave_x(Wave w, float2 p, float t) {
	return w.A * w.q * w.d.x * cos_wave_f(w, p, t);
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

// ------- INPUT / OUTPUT ---

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

// ----- VERTEX SHADER -----

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;

	const float2 p = i.pos.xy;

	float x = p.x;
	float y = p.y;
	float height = 0;

	float nx = 0;
	float ny = 0;
	float nz = 1;

	for (int w_i = 0; w_i < N_WAVES; w_i++) {
		Wave wv = w[w_i];

		x += sample_wave_x(wv, p, TIME);
		y += sample_wave_y(wv, p, TIME);
		height += sample_wave_height(wv, p, TIME);

		nx -= sample_wave_nx(wv, p, TIME);
		ny -= sample_wave_ny(wv, p, TIME);
		nz -= sample_wave_nz(wv, p, TIME);
	}

	float4 pos = float4(x, height, y, 1.0);
	o.world_pos = mul(MODEL, pos);
	o.screen_pos = mul(MVP, pos);

	float3 normal = normalize(float3(nx, nz, ny));
	float3x3 btn = {
		i.b,
		i.t,
		i.n
	};
	o.n = float4(normalize(mul(btn, normal)), 0);
	o.n = mul(MODEL, o.n);
	o.n = mul(VIEW, o.n);

	o.uv = i.uv;
	return o;
}

// ------- PIXEL SHADER ---------

float4 PS_main(VS_OUT i) : SV_TARGET {
	float ndotl = max(0.0, dot(i.n.xyz, -direction));
float3 shade = ndotl * intensity;

float4 view_pos = float4(VIEW[3][0], VIEW[3][1], VIEW[3][2], 1);
float4 view_dir = normalize(view_pos - i.world_pos);
float3 reflected = 2 * ndotl * i.n.xyz + direction;
float view_angle = max(0.0, dot(view_dir, float4(reflected, 0)));
float4 spec = pow(view_angle, 1000);

float4 color = float4(i.uv, 0, 1);
return float4(shade*(color.rgb + spec.rgb), 1);
}