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

cbuffer material : register(b5) {
	float roughness;
	float reflectivity;
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

float calculate_falloff(float r) {
	return 1 / pow(r, 2);
}

inline half4 Pow5(half4 x) {
	return x*x * x*x * x;
}

inline half3 FresnelTerm(half3 F0, half cosA) {
	half t = Pow5(1 - cosA);   // ala Schlick interpoliation
	return F0 + (1 - F0) * t;
}

inline half3 FresnelLerp(half3 F0, half3 F90, half cosA) {
	half t = Pow5(1 - cosA);   // ala Schlick interpoliation
	return lerp(F0, F90, t);
}

float disneyDiffuse(float NdotV, float NdotL, float LdotH, float perceptualRoughness) {
	float fd90 = 0.5 + 2 * LdotH * LdotH * perceptualRoughness;
	// Two schlick fresnel term
	float lightScatter = (1 + (fd90 - 1) * Pow5(1 - NdotL));
	float viewScatter = (1 + (fd90 - 1) * Pow5(1 - NdotV));

	return lightScatter * viewScatter;
}

float ndf(float ndoth, float roughness) {
	// GGX
	float r2 = roughness*roughness;
	float d  = (ndoth * r2 - ndoth) * ndoth + 1.0f;
	return r2 / (d*d + 1e-7f) * (1 / 3.1415926f);
}

float gsf(float NdotL, float NdotV, float roughness) {
	// SmithJointGGX
	float a = roughness;
	float lambdaV = NdotL * (NdotV * (1 - a) + a);
	float lambdaL = NdotV * (NdotL * (1 - a) + a);

	return 0.5f / (lambdaV + lambdaL + 1e-5f);
	
	/*float roughnessSqr = roughness*roughness;


	float SmithL = (NdotL) / (NdotL * (1 - roughnessSqr) + roughnessSqr);
	float SmithV = (NdotV) / (NdotV * (1 - roughnessSqr) + roughnessSqr);


	return (SmithL * SmithV);*/
	
	/*float roughnessSqr = roughness*roughness;
	float NdotLSqr = NdotL*NdotL;
	float NdotVSqr = NdotV*NdotV;


	float SmithL = (2 * NdotL) / (NdotL + sqrt(roughnessSqr +
		(1 - roughnessSqr) * NdotLSqr));
	float SmithV = (2 * NdotV) / (NdotV + sqrt(roughnessSqr +
		(1 - roughnessSqr) * NdotVSqr));


	float Gs = (SmithL * SmithV);
	return Gs;*/
}

float4 PS_main(VS_OUT i) : SV_TARGET {

	float3 t_albedo   = albedo.Sample(ss, i.uv).xyz;
	float t_world     = world.Sample(ss, i.uv).x;
	float3 t_normal   = normal.Sample(ss, i.uv).xyz;
	float3 t_specular = specular.Sample(ss, i.uv).xyz;

	float3 world_pos = i.frustum_dir * (t_world / i.frustum_dir.z);
	if (world_pos.z == 0) return float4(0, 0, 0, 1);
	
	float3 s_to_l    = normalize(light_position - world_pos);

	float3 s_to_v = -normalize(i.frustum_dir);
	float3 h = normalize((s_to_v + s_to_l) * 0.5);
	
	float3 ndotl = saturate(dot(s_to_l, t_normal));
	float ndoth  = saturate(dot(t_normal, h));
	float ndotv  = saturate(dot(t_normal, s_to_v));
	float ldoth  = saturate(dot(s_to_l, h));

	//float perceptualRoughness = SmoothnessToPerceptualRoughness(smoothness);
	//float roughness = PerceptualRoughnessToRoughness(perceptualRoughness);

	float diffuseTerm = disneyDiffuse(ndotv, ndotl, ldoth, sqrt(roughness)) * ndotl;

	float D = ndf(ndoth, roughness);
	float V = gsf(ndotl, ndotv, roughness);
	float specularTerm = D * V * 3.14159;
	specularTerm = max(0, specularTerm * ndotl);

	float surfaceReduction = 1.0 / (roughness*roughness + 1.0);
	float grazingTerm = saturate((1 - roughness) + (reflectivity));

	float3 diffColor = t_albedo;
	float3 specColor = t_specular;
	float3 c = 
		diffColor * (light_color * diffuseTerm) + 
		specularTerm * light_color * FresnelTerm(specColor, ldoth) + 
		surfaceReduction * FresnelLerp(specColor, grazingTerm, ndotv);

	float light_falloff = calculate_falloff(length(light_position - world_pos));
	return float4(c * light_intensity * light_falloff, 1);

	/*float dist_to_l  = length(s_to_l);
	float3 c = 
		dot(normalize(s_to_l), t_normal) * 
		light_intensity * 
		calculate_falloff(length(s_to_l)) *
		light_color *
		t_albedo
		; */

	//return float4(c, 1);
}