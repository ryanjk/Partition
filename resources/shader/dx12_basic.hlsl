//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct PSInput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

PSInput VSMain(VSInput input) {
	PSInput result;

	result.position = float4(input.position, 1.0f);
	result.normal = input.normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(1.0, 0.0, 0.0, 1.0);
}