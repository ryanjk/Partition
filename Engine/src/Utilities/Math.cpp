#include <Utilities\Math.h>
#include <Utilities\Logging.h>

#include <utility>

namespace pn {

// ------- CONSTANTS -------

const float EPSILON = 0.00001f;

const vec2f vec2f::Zero		= vec2f( 0.0f, 0.0f );
const vec2f vec2f::One		= vec2f(1.0f, 1.0f);
const vec2f vec2f::UnitX	= vec2f(1.0f, 0.0f);
const vec2f vec2f::UnitY	= vec2f(0.0f, 1.0f);

const vec3f vec3f::Zero		= vec3f(0.0f, 0.0f, 0.0f);
const vec3f vec3f::One		= vec3f(1.0f, 1.0f, 1.0f);
const vec3f vec3f::UnitX	= vec3f(1.0f, 0.0f, 0.0f);
const vec3f vec3f::UnitY	= vec3f(0.0f, 1.0f, 0.0f);
const vec3f vec3f::UnitZ	= vec3f(0.0f, 0.0f, 1.0f);

const vec4f vec4f::Zero		= vec4f(0.0f, 0.0f, 0.0f, 0.0f);
const vec4f vec4f::One		= vec4f(1.0f, 1.0f, 1.0f, 1.0f);
const vec4f vec4f::UnitX	= vec4f(1.0f, 0.0f, 0.0f, 0.0f);
const vec4f vec4f::UnitY	= vec4f(0.0f, 1.0f, 0.0f, 0.0f);
const vec4f vec4f::UnitZ	= vec4f(0.0f, 0.0f, 1.0f, 0.0f);
const vec4f vec4f::UnitW	= vec4f(0.0f, 0.0f, 0.0f, 1.0f);

const mat4f mat4f::Identity = mat4f(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
const mat4f mat4f::Zero		= mat4f(0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f);
const mat4f mat4f::One		= mat4f(1.0f, 1.0f, 1.0f, 1.0f,
									1.0f, 1.0f, 1.0f, 1.0f,
									1.0f, 1.0f, 1.0f, 1.0f,
									1.0f, 1.0f, 1.0f, 1.0f);

// ------ FUNCTIONS --------

// -------- EQUALITY TESTING ------------

bool IsEqual(const vec2f& v1, const vec2f& v2, const float eps = EPSILON) {
	return (abs(v1.x - v2.x) <= EPSILON) && 
		(abs(v1.y - v2.y) <= EPSILON);
}
bool IsEqual(const vec3f& v1, const vec3f& v2, const float eps = EPSILON) {
	return (abs(v1.x - v2.x) <= EPSILON) && 
		(abs(v1.y - v2.y) <= EPSILON) && 
		(abs(v1.z - v2.z) <= EPSILON);
}
bool IsEqual(const vec4f& v1, const vec4f& v2, const float eps = EPSILON) {
	return (abs(v1.x - v2.x) <= EPSILON) &&
		(abs(v1.y - v2.y) <= EPSILON) &&
		(abs(v1.z - v2.z) <= EPSILON) &&
		(abs(v1.w - v2.w) <= EPSILON);
}
#define E(x,y,e) (abs(x-y) <= e)
bool IsEqual(const mat4f& v1, const mat4f& v2, const float eps = EPSILON) {
	return
		E(v1._00, v2._00, eps) &&
		E(v1._01, v2._01, eps) &&
		E(v1._02, v2._02, eps) &&
		E(v1._03, v2._03, eps) &&
		E(v1._10, v2._10, eps) &&
		E(v1._11, v2._11, eps) &&
		E(v1._12, v2._12, eps) &&
		E(v1._13, v2._13, eps) &&
		E(v1._20, v2._20, eps) &&
		E(v1._21, v2._21, eps) &&
		E(v1._22, v2._22, eps) &&
		E(v1._23, v2._23, eps) &&
		E(v1._30, v2._30, eps) &&
		E(v1._31, v2._31, eps) &&
		E(v1._32, v2._32, eps) &&
		E(v1._33, v2._33, eps);
}
#undef E
// -------- OTHER FUNCTIONS ------------

vec3f	Cross(const vec3f& u, const vec3f& v) {
	vec3f result;
	result.x = (u.y*v.z) - (u.z*v.y);
	result.y = (u.z*v.x) - (u.x*v.z);
	result.z = (u.x*v.y) - (u.y*v.x);
	return result;
}

#define OP(x,m1,m2) ((x < m1) ? m1 : (x > m2) ? m2 : x)
float	Clamp(const float u, const float min, const float max) {
	return OP(u, min, max);
}
vec2f	Clamp(const vec2f& u, const vec2f& min, const vec2f& max) {
	vec2f result;
	result.x = OP(u.x, min.x, max.x);
	result.y = OP(u.y, min.y, max.y);
	return result;
}
vec3f	Clamp(const vec3f& u, const vec3f& min, const vec3f& max) {
	vec3f result;
	result.x = OP(u.x, min.x, max.x);
	result.y = OP(u.y, min.y, max.y);
	result.z = OP(u.z, min.z, max.z);
	return result;
}
vec4f	Clamp(const vec4f& u, const vec4f& min, const vec4f& max) {
	vec4f result;
	result.x = OP(u.x, min.x, max.x);
	result.y = OP(u.y, min.y, max.y);
	result.z = OP(u.z, min.z, max.z);
	result.z = OP(u.w, min.w, max.w);
	return result;
}
#undef OP

#define OP(x,y) ((x < y) ? x : y)
vec2f	Min(const vec2f& u, const vec2f& v) {
	vec2f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	return result;
}
vec3f	Min(const vec3f& u, const vec3f& v) {
	vec3f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	result.z = OP(u.z, v.z);
	return result;
}
vec4f	Min(const vec4f& u, const vec4f& v) {
	vec4f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	result.z = OP(u.z, v.z);
	result.w = OP(u.w, v.w);
	return result;
}
#undef OP

#define OP(x,y) ((x > y) ? x : y)
vec2f	Max(const vec2f& u, const vec2f& v) {
	vec2f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	return result;
}
vec3f	Max(const vec3f& u, const vec3f& v) {
	vec3f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	result.z = OP(u.z, v.z);
	return result;
}
vec4f	Max(const vec4f& u, const vec4f& v) {
	vec4f result;
	result.x = OP(u.x, v.x);
	result.y = OP(u.y, v.y);
	result.z = OP(u.z, v.z);
	result.w = OP(u.w, v.w);
	return result;
}
#undef OP

template<>
float			SmoothStep(const float& edge0, const float& edge1, const float& v) {
	float t = Clamp((v - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return t*t*(3.0f - 2.0f*t);
}

// ------- MATRIX FUNCTIONS ----------

mat4f Transpose(const mat4f& m) {
	return mat4f(
		m._00, m._10, m._20, m._30,
		m._01, m._11, m._21, m._31,
		m._02, m._12, m._22, m._32,
		m._03, m._13, m._23, m._33
	);
}

mat4f Inverse(const mat4f& in) {
	// from https://stackoverflow.com/questions/2624422/efficient-4x4-matrix-inverse-affine-transform
	mat4f m;
	float s0 = in._00 * in._11 - in._10 * in._01;
	float s1 = in._00 * in._12 - in._10 * in._02;
	float s2 = in._00 * in._13 - in._10 * in._03;
	float s3 = in._01 * in._12 - in._11 * in._02;
	float s4 = in._01 * in._13 - in._11 * in._03;
	float s5 = in._02 * in._13 - in._12 * in._03;

	float c5 = in._22 * in._33 - in._32 * in._23;
	float c4 = in._21 * in._33 - in._31 * in._23;
	float c3 = in._21 * in._32 - in._31 * in._22;
	float c2 = in._20 * in._33 - in._30 * in._23;
	float c1 = in._20 * in._32 - in._30 * in._22;
	float c0 = in._20 * in._31 - in._30 * in._21;

	// Should check for 0 determinant

	float invdet = 1 / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

	m._00 = (in._11 * c5 - in._12 * c4 + in._13 * c3) * invdet;
	m._01 = (-in._01 * c5 + in._02 * c4 - in._03 * c3) * invdet;
	m._02 = (in._31 * s5 - in._32 * s4 + in._33 * s3) * invdet;
	m._03 = (-in._21 * s5 + in._22 * s4 - in._23 * s3) * invdet;

	m._10 = (-in._10 * c5 + in._12 * c2 - in._13 * c1) * invdet;
	m._11 = (in._00 * c5 - in._02 * c2 + in._03 * c1) * invdet;
	m._12 = (-in._30 * s5 + in._32 * s2 - in._33 * s1) * invdet;
	m._13 = (in._20 * s5 - in._22 * s2 + in._23 * s1) * invdet;

	m._20 = (in._10 * c4 - in._11 * c2 + in._13 * c0) * invdet;
	m._21 = (-in._00 * c4 + in._01 * c2 - in._03 * c0) * invdet;
	m._22 = (in._30 * s4 - in._31 * s2 + in._33 * s0) * invdet;
	m._23 = (-in._20 * s4 + in._21 * s2 - in._23 * s0) * invdet;

	m._30 = (-in._10 * c3 + in._11 * c1 - in._12 * c0) * invdet;
	m._31 = (in._00 * c3 - in._01 * c1 + in._02 * c0) * invdet;
	m._32 = (-in._30 * s3 + in._31 * s1 - in._32 * s0) * invdet;
	m._33 = (in._20 * s3 - in._21 * s1 + in._22 * s0) * invdet;

	return m;
}

mat4f Translation(const vec3f& translation) {
	return Translation(translation.x, translation.y, translation.z);
}
mat4f Translation(const float xt, const float yt, const float zt) {
	return mat4f(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		xt  , yt  , zt  , 1.0f
	);
}

mat4f Scale(const float scale) {
	return Scale(scale, scale, scale);
}
mat4f Scale(const vec3f& scale) {
	return Scale(scale.x, scale.y, scale.z);
}
mat4f Scale(const float xs, const float ys, const float zs) {
	return mat4f(
		xs, 0.0f, 0.0f, 0.0f,
		0.0f, ys, 0.0f, 0.0f,
		0.0f, 0.0f, zs, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

vec3f AxisAngleToEuler(const vec3f& p_axis, float angle) {
	auto axis = p_axis;
	//axis.Normalize();
	float s = sinf(angle);
	float t = 1 - cosf(angle);
	float z_comp = (axis.x * axis.y * t) + (axis.z * s);
	float bank, heading, attitude;
	if (abs(z_comp) > 0.999) {
		float sign = z_comp < 0.0f ? -1.0f : 1.0f;
		bank = 0;
		heading = sign * 2 * atan2(axis.x*sinf(angle / 2), cosf(angle / 2));
		attitude = sign*DirectX::XM_PIDIV2;
	}
	else {
		bank = atan2(axis.x*s - axis.y*axis.z*t, 1 - (powf(axis.z, 2) + powf(axis.x, 2))*t);
		heading = atan2(axis.y*s - axis.x*axis.z*t, 1 - (powf(axis.y, 2) + powf(axis.z, 2))*t);
		attitude = asinf(z_comp);
	}
	return vec3f(bank, heading, attitude);
}

/*
void TestAngleToEuler() {
	auto test = [](float x, float y, float z) {
		auto quat = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(y, x, z);
		float angle;
		DirectX::XMVECTOR axis;
		DirectX::XMQuaternionToAxisAngle(&axis, &angle, quat);
		
		auto c = AxisAngleToEuler(pn::vec3f(DirectX::XMVectorGetByIndex(axis,0), DirectX::XMVectorGetByIndex(axis, 1), DirectX::XMVectorGetByIndex(axis, 2)), 
								  angle);
		//auto quat_c = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(c.y, c.x, c.z);
		
		Log("({}, {}, {}) | ({}, {}, {})", x, y, z, c.x, c.y, c.z);
		assert(abs(x - c.x) < 0.00001);
		assert(abs(y - c.y) < 0.00001);
		assert(abs(z - c.z) < 0.00001);
		const float EPS = 0.1;
		if (
			(abs(x - c.x) > EPS) ||
			(abs(y - c.y) > EPS) ||
			(abs(z - c.z) > EPS)) {
			Log("({}, {}, {}) | ({}, {}, {})", x, y, z, c.x, c.y, c.z);
		}
	};

	const float dt = 0.1f;
	for (float x = 0.0f; x < DirectX::XM_2PI; x += dt) {
		for (float y = 0.0f; y < DirectX::XM_2PI; y += dt) {
			for (float z = 0.0f; z < DirectX::XM_2PI; z += dt) {
				test(x, y, z);
			}
		}
	}
}
*/
} //namespace pn