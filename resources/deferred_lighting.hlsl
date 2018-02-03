#include "GlobalConstants.hlsli"

Texture2D albedo   : register(t1);
Texture2D world    : register(t2);
Texture2D normal   : register(t3);
Texture2D specular : register(t4);

SamplerState ss	: register(s1);

cbuffer light : register(b4) {
	float3 light_position;
	float3 light_color; float _padding;
	float  light_intensity;
}

// ----- INPUT / OUTPUT --------

struct VS_IN {
	uint vertex_id : SV_VertexID;
};

struct VS_OUT {
	float4 screen_pos  : SV_POSITION;
	float2 uv		   : TEXCOORD0;
	float3 frustum_dir : TEXCOORD1;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	float x = 779.98;
	float y = 1573.57;
	float z = 1000;
	if (i.vertex_id == 0) {
		o.uv         = float2(0, 0);
		o.screen_pos = float4(-1, 1, 0, 1);
		o.frustum_dir = float3(-x, y, z);
	}
	else if (i.vertex_id == 1) {
		o.uv = float2(1, 0);
		o.screen_pos = float4(1, 1, 0, 1);
		o.frustum_dir = float3(x, y, z);
	}
	else if (i.vertex_id == 2) {
		o.uv = float2(0, 1);
		o.screen_pos = float4(-1, -1, 0, 1);
		o.frustum_dir = float3(-x, -y, z);
	}
	else if (i.vertex_id == 3) {
		o.uv = float2(1, 1);
		o.screen_pos = float4(1, -1, 0, 1);
		o.frustum_dir = float3(x, -y, z);
	}
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET {
	float3 t_albedo   = albedo.Sample(ss, i.uv).xyz;
	float t_world     = world.Sample(ss, i.uv).x;
	float3 t_normal   = normalize(normal.Sample(ss, i.uv).xyz);
	float3 t_specular = specular.Sample(ss, i.uv).xyz;

	float3 world_pos = i.frustum_dir * (t_world / i.frustum_dir.z);
	float3 s_to_l = light_position - world_pos;
	float dist_to_l = length(s_to_l);
	float3 c = 
		dot(normalize(s_to_l), t_normal) * 
		light_intensity * 
		(1/pow(dist_to_l,2)) *
		light_color
		;

	return float4(c, 1);
}