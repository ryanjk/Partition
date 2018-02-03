#include "GlobalConstants.hlsli"

Texture2D albedo   : register(t1);
Texture2D world    : register(t2);
Texture2D normal   : register(t3);
Texture2D specular : register(t4);

SamplerState ss	: register(s1);

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

float  camera_near = 0.01;
float  camera_far = 1000;

float viewportToWorld(float z) {
	float A = camera_far / (camera_far - camera_near);                                             
	float B = -1 * A * camera_near;
	return B / (z - A);
}

float4 PS_main(VS_OUT i) : SV_TARGET {
	float3 t_albedo   = albedo.Sample(ss, i.uv).xyz;
	float t_world     = world.Sample(ss, i.uv).x;
	float3 t_normal   = normal.Sample(ss, i.uv).xyz;
	float3 t_specular = specular.Sample(ss, i.uv).xyz;

	const float  LIGHT_POW = 20.0;
	const float3 LIGHT_POS = float3(2, 2, 3);

	float3 world_pos = i.frustum_dir * (t_world / i.frustum_dir.z);
	float dist_to_l = length(LIGHT_POS - world_pos);
	float3 s_to_l = normalize(LIGHT_POS - world_pos);
	float3 c = dot(s_to_l, t_normal) * LIGHT_POW * (1/pow(dist_to_l,2));

	return float4(c, 1);
}