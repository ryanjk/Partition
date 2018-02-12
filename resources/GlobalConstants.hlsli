cbuffer _error : register(b0) {} // make sure D3D doesn't assign this register to any other buffer as it's used as an error value

cbuffer global_constants {
	float TIME;
	float SCREEN_WIDTH;
	float SCREEN_HEIGHT;
}

cbuffer camera_constants {
	float4x4 VIEW;
	float4x4 INV_VIEW;
	float4x4 PROJECTION;
	float4x4 INV_PROJECTION;
	float4x4 INV_PROJECTION_VIEW;
}

cbuffer model_constants {
	float4x4 MODEL;
	float4x4 MODEL_VIEW;
	float4x4 MODEL_VIEW_INVERSE_TRANSPOSE;
	float4x4 MVP;
}
