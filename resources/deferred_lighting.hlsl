#include "ShaderUtil.hlsli"
#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

Texture2D albedo   : register(t1);
Texture2D world    : register(t2);
Texture2D normal   : register(t3);
Texture2D specular : register(t4);

SamplerState ss	: register(s1);

cbuffer light {
	float3 light_position;
	float3 light_color; float _padding;
	float  light_intensity;
}

struct VS_OUT {
	float4 screen_pos  : SV_POSITION;
	float2 uv		   : TEXCOORD0;
	float3 frustum_dir : TEXCOORD1;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN_SCREEN_FULL i) {
	VS_OUT o;
	o.screen_pos  = float4(i.pos,1);
	o.frustum_dir = i.frustum_dir;
	o.uv          = (float2(1, 1) + i.pos.xy) * 0.5;
	o.uv.y        = 1 - o.uv.y;
	return o;
}

// ----- PIXEL SHADER -------

float4 PS_main(VS_OUT i) : SV_TARGET {

	// --- Read material data ---
	float4 t_albedo   = albedo.Sample(ss, i.uv);
	float  t_world    = world.Sample(ss, i.uv).x;
	float3 t_normal   = normal.Sample(ss, i.uv).xyz;
	float4 t_specular = specular.Sample(ss, i.uv);

	float3 m_albedo    = t_albedo.rgb;
	float metallic     = t_albedo.w;
	float roughness    = t_specular.w;
	float sqrRoughness = roughness*roughness;

	// --- Calculate normal, light and view vector products and validate ---

	if (t_world == 0.0f) return float4(0.2, 0.2, 0.2, 1);

	float3 vpos = float3(i.uv.x * 2 - 1, (1 - i.uv.y) * 2 - 1, t_world);
	float4 wp;
	wp = mul(INV_PROJECTION_VIEW, float4(vpos, 1));
	float3 world_pos = wp.xyz / wp.w;

	float3 camera_pos = -float3(VIEW[3][0], VIEW[3][1], VIEW[3][2]);
	float3 s_to_l = normalize(light_position - world_pos);
	float3 s_to_v = -normalize(i.frustum_dir - camera_pos);

	float ndotl = dot(s_to_l, t_normal);
	float ndotv = dot(t_normal, s_to_v);
	if (ndotl < 0 || ndotv < 0) return float4(0, 0, 0, 1);

	float3 h    = normalize(s_to_v + s_to_l);
	float ndoth = saturate(dot(t_normal, h));
	float ldoth = saturate(dot(s_to_l, h));
	
	// --- Calculate specular color ---
	const float3 DEFAULT_SPEC_COLOR = float3(0.04, 0.04, 0.04);
	float3 Cs = lerp(DEFAULT_SPEC_COLOR, m_albedo, metallic);
	float FH = SchlickFresnel(ldoth);
	float3 Fs = lerp(Cs, float3(1, 1, 1), FH);

	float Ds  = NDF_GGX(ndoth, max(0.001, sqrRoughness));
	float Gs  = GSF_GGX(ndotl, ndotv, max(0.001, roughness));
	float3 specColor = Fs * Ds * Gs / (4*ndotl*ndotv);

	// --- Calculate diffuse fresnel term ---
	float FL   = SchlickFresnel(ndotl);
	float FV   = SchlickFresnel(ndotv);
	float fd90 = 0.5 + 2.0 * ldoth * ldoth * roughness;
	float Fd   = lerp(1.0, fd90, FL) * lerp(1.0, fd90, FV);
	
	float3 diffuseColor = INV_PI * Fd * m_albedo * (1 - metallic);
	
	// --- Calculate final color ---
	float3 finalColor = (diffuseColor + specColor) *
		ndotl *
		light_intensity *
		LightFalloff(length(light_position - world_pos), 30) *
		light_color
		;
	return float4(finalColor, 1);
}