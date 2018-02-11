#define PI            3.14159265359f
#define TWO_PI        6.28318530718f
#define FOUR_PI       12.56637061436f
#define INV_PI        0.31830988618f
#define INV_TWO_PI    0.15915494309f
#define INV_FOUR_PI   0.07957747155f
#define HALF_PI       1.57079632679f
#define INV_HALF_PI   0.636619772367f

// --- General functions ---

float Pow5(float x) {
	return x*x * x*x * x;
}

// --- Lighting functions ---

float LightFalloff(float r, float light_radius) {
	return 1 / pow(r, 2);
	//float n = pow(saturate(1 - pow(r / light_radius, 4)), 2);
	//return n / ((r*r) + 1);
}

float SchlickFresnel(float u) {
	float m = clamp(1 - u, 0, 1);
	float m2 = m*m;
	return m2*m2*m; // pow(m,5)
}

// --- BRDF Functions ---

/*
https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
Specular D
*/
float NDF_GGX(float ndoth, float roughness) {
	float r2 = roughness*roughness;
	float d = pow(ndoth, 2) * (r2 - 1) + 1;
	return r2 / (d*d) * INV_PI;
}

/*
https://de45xmedrsdbp.cloudfront.net/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
Specular G
*/
float GSF_GGX_G1(float ndot, float k) {
	return ndot / (ndot * (1 - k) + k);
}

float GSF_GGX(float ndotl, float ndotv, float roughness) {
	float k = pow((roughness + 1), 2) / 8;
	return GSF_GGX_G1(ndotl, k) * GSF_GGX_G1(ndotv, k);
}