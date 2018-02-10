#define POSITION_DEF  float3 pos : POSITION
#define NORMAL_DEF    float3 n   : NORMAL
#define TANGENT_DEF   float3 t   : TANGENT
#define BITANGENT_DEF float3 b   : TANGENT1
#define UV_DEF        float2 uv  : TEXCOORD0

#define VERTEX_ID_DEF uint vertex_id : SV_VertexID

struct VS_IN_SIMPLE {
	POSITION_DEF;
	NORMAL_DEF;
	UV_DEF;
};

struct VS_IN_FULL {
	POSITION_DEF;
	NORMAL_DEF;
	TANGENT_DEF;
	BITANGENT_DEF;
	UV_DEF;
};

struct VS_IN_SCREEN {
	VERTEX_ID_DEF;
};

