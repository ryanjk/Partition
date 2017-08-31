#include "GlobalConstants.hlsli"

Texture2D normal_map	: register(t1);
Texture2D diffuse_map	: register(t2);
Texture2D height_map	: register(t3);

SamplerState ss			: register(s1);

// ---- CONSTANT BUFFERS --------

cbuffer directional_light : register(b4) {
	float3	direction;
	float	intensity;
}

cbuffer mapping_vars : register(b5) {
	float height_scale;
};

// ----- INPUT / OUTPUT --------

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
	float2 uv			: TEXCOORD0;
	float3 n			: TEXCOORD1;
	float3 t			: TEXCOORD2;
	float3 b			: TEXCOORD3;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	float4 pos		= float4(i.pos, 1.0);
	pos				= mul(MODEL, pos);
	o.world_pos		= pos;
	pos				= mul(VIEW, pos);
	o.screen_pos	= mul(PROJECTION, pos);

	o.n = i.n;
	o.t = i.t;
	o.b = i.b;

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float3 vis(float3 vec) {
	return (vec + float3(1, 1, 1))*0.5;
}

float4 show(float3 vec) { return float4(vec, 1); }

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 view_pos = float3(VIEW[3][0], VIEW[3][1], VIEW[3][2]);
	float3 view_dir = normalize(view_pos - i.world_pos.xyz);

	float height = height_map.Sample(ss, i.uv).x;

	float3x3 btn	= transpose(float3x3(i.t, i.b, i.n));
	float3 uv_offset = mul(transpose(btn), view_dir);

	i.uv = i.uv - uv_offset.xy / uv_offset.z * (height * height_scale);

	float3 nmapn	= normal_map.Sample(ss, i.uv).xyz;
	nmapn			= normalize((2 * nmapn) - float3(1, 1, 1));

	float3 n		= i.n;
	n				= mul(btn, nmapn);
	n				= mul(MODEL,n);
	//return float4(n, 1);

	float ndotl = saturate(dot(n, -direction));
	float3 shade = ndotl * intensity;

#define USE_DIFFUSE_TEXTURE
#ifdef USE_DIFFUSE_TEXTURE
	float3 color = diffuse_map.Sample(ss, i.uv);
#else
	float3 color = float3(.1, .1, .1);
#endif

	float3 halfw = normalize(view_dir - direction);
	float4 spec = pow(saturate(dot(n,halfw)), 10);

	return float4(shade*(color.rgb+0.5*spec.rgb), 1);
}