Texture2D world    : register(t2);
Texture2D normal   : register(t3);

#define USE_MATERIAL_TEX

#ifdef USE_MATERIAL_TEX
Texture2D albedo   : register(t1);
Texture2D specular : register(t4);
#else
cbuffer material {
	float4 albedo;
	float4 specular;
}
#endif