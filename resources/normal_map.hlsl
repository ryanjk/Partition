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
	float height_bias;
};

// ----- INPUT / OUTPUT --------

struct VS_IN {
	float3 model_normal		: NORMAL;
	float3 model_tangent	: TANGENT;
	float3 model_bitangent	: TANGENT1;
	float3 model_pos		: POSITION;
	float2 uv				: TEXCOORD0;
};

struct VS_OUT {
	float4 screen_pos		: SV_POSITION;
	float2 uv				: TEXCOORD0;
	float3 tspace_to_light	: TEXCOORD1;
	float3 tspace_to_camera : TEXCOORD2;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN i) {
	VS_OUT o;
	float4 pos			= float4(i.model_pos, 1.0);
	pos					= mul(MODEL, pos);
	float4 world_pos	= pos;
	pos					= mul(VIEW, pos);
	o.screen_pos		= mul(PROJECTION, pos);

	float3 world_normal		= normalize(mul(MODEL,i.model_normal));
	float3 world_tangent	= normalize(mul(MODEL,i.model_tangent));
	float3 world_bitangent	= normalize(mul(MODEL,i.model_bitangent));

	float3 world_to_light	= -direction;
	o.tspace_to_light		= float3(
		dot(world_tangent,   world_to_light),
		dot(world_bitangent, world_to_light),
		dot(world_normal,    world_to_light)
	);

	float3 world_camera_pos	= float3(VIEW[3][0], VIEW[3][1], VIEW[3][2]);
	float3 world_to_camera	= normalize(world_camera_pos - world_pos.xyz);
	o.tspace_to_camera = float3(
		dot(world_tangent,   world_to_camera),
		dot(world_bitangent, world_to_camera),
		dot(world_normal,    world_to_camera)
	);

	o.uv = i.uv;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET{

	float3 tspace_view	= normalize(i.tspace_to_camera);
	float3 uvh = float3(i.uv.xy, 0);
	uvh.x = 1 - uvh.x;
	for (int c = 0; c < 1000; c++) {
		float height	= height_map.Sample(ss, uvh.xy).x;
		height			= height * height_scale + height_bias;
		uvh				+= (tspace_view) * (height - uvh.z);
	}

	i.uv.xy = uvh.xy;
	if (i.uv.x > 1 || i.uv.x < 0 || i.uv.y > 1 || i.uv.y < 0) {
		discard;
	}

	float3 nmapn = normal_map.Sample(ss, i.uv).xyz;
	float3 n	 = normalize((2 * nmapn) - float3(1, 1, 1));

	float3 tspace_light = normalize(i.tspace_to_light);
	float ndotl			= saturate(dot(n, tspace_light));
	float3 shade		= ndotl * intensity;

	float3 color = diffuse_map.Sample(ss, i.uv);

	float3 halfw = normalize(tspace_view + tspace_light);
	float4 spec = pow(saturate(dot(n,halfw)), 1000);

	return float4(shade*(color.rgb+spec.rgb), 1);
}