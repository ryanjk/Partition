cbuffer _error : register(b0) {} // make sure D3D doesn't assign this register to any other buffer as it's used as an error value

cbuffer global_constants : register(b1) {
	float TIME;
	float SCREEN_WIDTH;
	float SCREEN_HEIGHT;
}

cbuffer camera_constants : register(b2) {
	float4x4 VIEW;
	float4x4 PROJECTION;
}

cbuffer model_constants : register(b3) {
	float4x4 MODEL;
	float4x4 MODEL_VIEW_INVERSE_TRANSPOSE;
	float4x4 MVP;
}