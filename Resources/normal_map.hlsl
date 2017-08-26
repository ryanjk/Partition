#include "GlobalConstants.hlsli"

Texture2D normal_map	: register(t1);
SamplerState ss			: register(s1);

// ---- CONSTANT BUFFERS --------

cbuffer directional_light : register(b4) {
	float3	direction;
	float	intensity;
}

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
	float4 pos = float4(i.pos, 1.0);
	pos = mul(MODEL, pos);
	o.world_pos = pos;
	pos = mul(VIEW, pos);
	o.screen_pos = mul(PROJECTION, pos);

	o.n = i.n;
	o.b = i.b;
	o.t = i.t;

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 n3 = normalize(i.n);
	float3x3 btn = transpose(float3x3(i.t, i.b, n3));
	float3 nmapn = normal_map.Sample(ss, i.uv).xyz;
	n3 = mul(btn, nmapn);
	
	float4 n4 = float4(n3, 0.0);
	n4 = mul(MODEL, n4);
	//n4 = mul(VIEW, n4);
	//return float4(n4.xyz, 1);
	n4 = normalize(n4);

	float3 n = n4.xyz;


	float ndotl = saturate(dot(n, -direction));
	float3 shade = ndotl * intensity;

	float3 view_pos = float3(VIEW[3][0], VIEW[3][1], VIEW[3][2]);
	float3 view_dir = normalize(view_pos - i.world_pos.xyz);
	float3 halfw = normalize(view_dir - direction);
	float4 spec = pow(saturate(dot(n,halfw)), 100);

	float4 color = float4(0.2, 1, 1, 1);
	return float4(shade*(color.rgb ), 1);
}