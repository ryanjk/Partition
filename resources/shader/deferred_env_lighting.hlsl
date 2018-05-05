#include "ShaderUtil.hlsli"
#include "GlobalConstants.hlsli"
#include "ShaderStructs.hlsli"

#include "DeferredShading.hlsli"

cbuffer environment_lighting {
	float environment_intensity;
}

TextureCube environment : register(t5);

SamplerState ss : register(s1) {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_OUT {
	float4 screen_pos  : SV_POSITION;
	float2 uv		   : TEXCOORD0;
};

// ------- VERTEX SHADER --------

VS_OUT VS_main(VS_IN_SCREEN i) {
	VS_OUT o;
	o.screen_pos = float4(i.pos, 1);
	o.uv = (float2(1, 1) + i.pos.xy) * 0.5;
	o.uv.y = 1 - o.uv.y;
	return o;
}

// ----- PIXEL SHADER -------

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float roughness, float3 n) {
	float a = roughness*roughness;

	float phi = 2 * PI * Xi.x;
	float cos_theta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
	float sin_theta = sqrt(1 - cos_theta * cos_theta);

	float3 H;
	H.x = sin_theta * cos(phi);
	H.y = sin_theta * sin(phi);
	H.z = cos_theta;

	float3 up = abs(n.z) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
	float3 tangentX = normalize(cross(up, n));
	float3 tangentY = cross(n, tangentX);

	return tangentX * H.x + tangentY * H.y + n * H.z;
}

float3 SampleEnvironment(float3 direction) {
	return environment.Sample(ss, direction, 0).rgb;
}

float4 PS_main(VS_OUT i) : SV_TARGET{

	// --- Read material data ---
	float  t_world    = world.Sample(ss, i.uv).x;
	float3 t_normal   = normal.Sample(ss, i.uv).xyz;
	float4 t_albedo   = albedo.Sample(ss, i.uv);
	float4 t_specular = specular.Sample(ss, i.uv);

	float3 m_albedo = t_albedo.rgb;
	float metallic = t_albedo.w;
	float roughness = t_specular.w;
	float3 ao = t_specular.xyz;
	float sqrRoughness = roughness*roughness;

	// --- Calculate normal, light and view vector products and validate ---
	float3 world_pos = WorldPosFromDepth(i.uv, t_world, INV_PROJECTION_VIEW);
	float3 camera_pos = float3(INV_VIEW[0][3], INV_VIEW[1][3], INV_VIEW[2][3]);
	float3 s_to_v = normalize(camera_pos - world_pos);

	// --- Sample environment
	const float3 DEFAULT_SPEC_COLOR = float3(0.04, 0.04, 0.04);
	float3 specColor = lerp(DEFAULT_SPEC_COLOR, m_albedo, metallic);
	float3 specular_lighting = float3(0, 0, 0);
	const uint n_samples = 256;
	for (uint i = 0; i < n_samples; i++) {
		float2 Xi = Hammersley(i, n_samples);
		float3 H = ImportanceSampleGGX(Xi, roughness, t_normal);
		float3 L = 2 * dot(s_to_v, H) * H - s_to_v;
		
		float ndotl = saturate(dot(t_normal, L));
		if (ndotl > 0) {
			float ndotv = saturate(dot(t_normal, s_to_v	));
			float ndoth = saturate(dot(t_normal, H));
			float vdoth = saturate(dot(s_to_v, H));

			float3 SampleColor = SampleEnvironment(L);
			float G = GSF_GGX(ndotl, ndotv, max(0.001, roughness));
			float Fc = pow(1 - vdoth, 5);
			float3 F = (1 - Fc) * specColor + Fc;

			specular_lighting += SampleColor * F * G * vdoth / (ndoth * ndotv);
			//specular_lighting += float3(0, 0, 1);
		}
		//else specular_lighting += float3(1, 0, 0);
	}

	return float4(specular_lighting / n_samples * environment_intensity, 1);
}