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

float  camera_near = 0.01;
float  camera_far = 1000;

float viewportToWorld(float z) {
	float A = camera_far / (camera_far - camera_near);                                             
	float B = -1 * A * camera_near;
	return B / (z - A);
}

float4 PS_main(VS_OUT i) : SV_TARGET{
	float3 t_albedo = albedo.Sample(ss, i.uv).xyz;
	float t_world = world.Sample(ss, i.uv).x;
	float3 t_normal = normal.Sample(ss, i.uv).xyz;
	float3 t_specular = specular.Sample(ss, i.uv).xyz;

	const float  LIGHT_POW = 1.0;
	const float3 LIGHT_POS = float3(2, 2, 3);

	float2 camera_uv = float2(i.uv.x, 1 - i.uv.y) * 2 - 1;
	camera_uv.x *= SCREEN_WIDTH / SCREEN_HEIGHT;

	float3 camera_pos = float3(0, 0, 0);
	float3 camera_x = float3(1, 0, 0);
	float3 camera_y = float3(0, 1, 0);
	float3 camera_z = float3(0, 0, 1);

	float3 projection_point = 
		camera_pos + 
		camera_near*camera_z + 
		camera_x*camera_uv.x*10 + 
		camera_y*camera_uv.y*10;
	float3 camera_to_point = (projection_point - camera_pos);
	float3 world_pos       = camera_to_point / camera_to_point.z * viewportToWorld(t_world);

	float3 p_to_l = LIGHT_POS - world_pos;

	float3 c = dot(t_normal, normalize(p_to_l))*LIGHT_POW;

	//return float4(projection_point, 1);

	return float4(world_pos.z/9.2, 0, 0, 1);
}