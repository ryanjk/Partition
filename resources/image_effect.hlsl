#include "GlobalConstants.hlsli"

Texture2D offscreen_texture : register(t1);

SamplerState ss			: register(s1);

// ----- INPUT / OUTPUT --------

struct VS_IN {
	uint vertex_id : SV_VertexID;
};

struct VS_OUT {
	float4 screen_pos : SV_POSITION;
	float2 uv		  : TEXCOORD0;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	if (i.vertex_id == 0) {
		o.uv         = float2(0, 0);
		o.screen_pos = float4(-1, 1, 0, 1);
	}
	else if (i.vertex_id == 1) {
		o.uv = float2(1, 0);
		o.screen_pos = float4(1, 1, 0, 1);
	}
	else if (i.vertex_id == 2) {
		o.uv = float2(0, 1);
		o.screen_pos = float4(-1, -1, 0, 1);
	}
	else if (i.vertex_id == 3) {
		o.uv = float2(1, 1);
		o.screen_pos = float4(1, -1, 0, 1);
	}
	return o;
}

// ----- PIXEL SHADER -------

cbuffer blur_params {
	float2 dir;
	float sigma;
};

float gaussian(float x) {
	return exp(-(pow(x, 2.0) / (pow(sigma, 2.0) * 2.0)));
}

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 tex = offscreen_texture.Sample(ss, i.uv).xyz;
	float2 res = float2(SCREEN_WIDTH, SCREEN_HEIGHT);
	float  ar  = res.x / res.y;

	float3 c      = float3(0, 0, 0);
	float2 offset = dir / res;

	const int RADIUS      = 67;
	const int NUM_WEIGHTS = RADIUS + 1;
	float w[NUM_WEIGHTS];
	float sum = 0;
	for (int x = 0; x < NUM_WEIGHTS; x++) {
		w[x] = gaussian(float(x));

		c += offscreen_texture.Sample(ss, i.uv + x * offset).xyz * w[x];
		c += offscreen_texture.Sample(ss, i.uv - x * offset).xyz * w[x];

		sum += 2 * w[x];
	}
	sum -= w[0];
	c   -= offscreen_texture.Sample(ss, i.uv).xyz * w[0];

	return float4(c / sum, 1);
}