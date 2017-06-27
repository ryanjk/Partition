#pragma once

#include <DirectXMath.h>

#include <functional>

namespace pn {

// ---------- VECTORS -------------

struct vec2f {
	float x;
	float y;

	vec2f() noexcept : x(), y() {}
	vec2f(float x, float y) noexcept : x(x), y(y) {}
	vec2f(const vec2f& vec) noexcept : x(vec.x), y(vec.y) {}

	vec2f operator+(const vec2f& rhs) const { return vec2f(x + rhs.x, y + rhs.y); }
	vec2f operator-(const vec2f& rhs) const { return vec2f(x - rhs.x, y - rhs.y); }
	vec2f operator*(const vec2f& rhs) const { return vec2f(x*rhs.x, y*rhs.y); }
	vec2f operator/(const vec2f& rhs) const { return vec2f(x / rhs.x, y / rhs.y); }

	vec2f& operator+=(const vec2f& rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2f& operator-=(const vec2f& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2f& operator*=(const vec2f& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	vec2f& operator/=(const vec2f& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	vec2f operator*(const float c) const { return vec2f(c*x, c*y); }
	vec2f operator/(const float c) const { return vec2f(x/c, y/c); }
	
	vec2f& operator*=(const float c) { x *= c; y *= c; return *this; }
	vec2f& operator/=(const float c) { x /= c; y /= c; return *this; }

	static const vec2f Zero;
	static const vec2f One;
	static const vec2f UnitX;
	static const vec2f UnitY;
};

struct vec3f {
	float x;
	float y;
	float z;

	vec3f() noexcept : x(), y(), z() {}
	vec3f(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
	vec3f(const vec3f& vec) noexcept : x(vec.x), y(vec.y), z(vec.z) {}

	vec3f(const vec2f& vec, float z) noexcept : x(vec.x), y(vec.y), z(z) {}
	vec3f(float x, const vec2f& vec) noexcept : x(x), y(vec.x), z(vec.y) {}

	vec3f operator+(const vec3f& rhs) const { return vec3f(x + rhs.x, y + rhs.y, z + rhs.z); }
	vec3f operator-(const vec3f& rhs) const { return vec3f(x - rhs.x, y - rhs.y, z - rhs.z); }
	vec3f operator*(const vec3f& rhs) const { return vec3f(x * rhs.x, y * rhs.y, z * rhs.z); }
	vec3f operator/(const vec3f& rhs) const { return vec3f(x / rhs.x, y / rhs.y, z / rhs.z); }

	vec3f& operator+=(const vec3f& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3f& operator-=(const vec3f& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	vec3f& operator*=(const vec3f& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	vec3f& operator/=(const vec3f& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }

	vec3f operator*(const float c) const { return vec3f(c*x, c*y, c*z); }
	vec3f operator/(const float c) const { return vec3f(x/c, y/c, z/c); }
	
	vec3f& operator*=(const float c) { x *= c; y *= c; z *= c; return *this; }
	vec3f& operator/=(const float c) { x /= c; y /= c; z /= c; return *this; }

	static const vec3f Zero;
	static const vec3f One;
	static const vec3f UnitX;
	static const vec3f UnitY;
	static const vec3f UnitZ;
};

struct vec4f {
	float x;
	float y;
	float z;
	float w;

	vec4f() noexcept : x(), y(), z(), w() {}
	vec4f(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
	vec4f(const vec4f& vec) noexcept : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}

	vec4f(const vec3f& xyz, float w) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
	vec4f(float x, const vec3f& yzw) noexcept : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}

	vec4f(const vec2f& xy, const vec2f& zw) noexcept : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
	vec4f(const vec2f& xy, float z, float w) noexcept : x(xy.x), y(xy.y), z(z), w(w) {}
	vec4f(float x, const vec2f& yz, float w) noexcept : x(x), y(yz.x), z(yz.y), w(w) {}
	vec4f(float x, float y, const vec2f& zw) noexcept : x(x), y(y), z(zw.x), w(zw.y) {}

	vec3f xyz() const { return vec3f(x, y, z); }

	vec4f operator+(const vec4f& rhs) const { return vec4f(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	vec4f operator-(const vec4f& rhs) const { return vec4f(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
	vec4f operator*(const vec4f& rhs) const { return vec4f(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
	vec4f operator/(const vec4f& rhs) const { return vec4f(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }

	vec4f& operator+=(const vec4f& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	vec4f& operator-=(const vec4f& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	vec4f& operator*=(const vec4f& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	vec4f& operator/=(const vec4f& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }

	vec4f operator*(const float c) const { return vec4f(c*x, c*y, c*z, c*w); }
	vec4f operator/(const float c) const { return vec4f(x / c, y / c, z / c, w / c); }

	vec4f& operator*=(const float c) { x *= c; y *= c; z *= c; w *= c; return *this; }
	vec4f& operator/=(const float c) { x /= c; y /= c; z /= c; w /= c; return *this; }

	static const vec4f Zero;
	static const vec4f One;
	static const vec4f UnitX;
	static const vec4f UnitY;
	static const vec4f UnitZ;
	static const vec4f UnitW;
};

// --------- QUATERNION ------------

struct quaternion	Inverse(const struct quaternion& q);
struct quaternion {
	float x;
	float y;
	float z;
	float w;

	quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	quaternion(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}
	explicit quaternion(const vec4f& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w) {}
	quaternion(const vec3f& vec, const float s) : x(vec.x), y(vec.y), z(vec.z), w(s) {}
	quaternion(const quaternion& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}

	quaternion operator+(const quaternion& rhs) const { return quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	quaternion operator-(const quaternion& rhs) const { return quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
	quaternion operator*(const quaternion& rhs) const { 
		return quaternion(
			w*rhs.x + x*rhs.w - y*rhs.z + z*rhs.y,
			w*rhs.y + x*rhs.z + y*rhs.w - z*rhs.x,
			w*rhs.z - x*rhs.y + y*rhs.x + z*rhs.w,
			w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z
		);
	}
	quaternion operator/(const quaternion& rhs) const { 
		return (*this) * Inverse(rhs);
	}

	quaternion& operator+=(const quaternion& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	quaternion& operator-=(const quaternion& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	quaternion& operator*=(const quaternion& rhs) { 
		*this = (*this) * rhs;
		return *this;
	}
	quaternion& operator/=(const quaternion& rhs) { 
		*this = (*this) / rhs;
		return *this; 
	}

	quaternion operator*(const float c) const { return quaternion(c*x, c*y, c*z, c*w); }
	quaternion operator/(const float c) const { return quaternion(x / c, y / c, z / c, w / c); }

	quaternion& operator*=(const float c) { x *= c; y *= c; z *= c; w *= c; return *this; }
	quaternion& operator/=(const float c) { x /= c; y /= c; z /= c; w /= c; return *this; }

	static const quaternion Zero;
	static const quaternion Identity;
};

// ---------- MATRIX ---------------

struct mat4f {
	float _00, _01, _02, _03;
	float _10, _11, _12, _13;
	float _20, _21, _22, _23;
	float _30, _31, _32, _33;

	mat4f() :
		_00(1.0f), _01(0.0f), _02(0.0f), _03(0.0f),
		_10(0.0f), _11(1.0f), _12(0.0f), _13(0.0f),
		_20(0.0f), _21(0.0f), _22(1.0f), _23(0.0f),
		_30(0.0f), _31(0.0f), _32(0.0f), _33(1.0f) {}

	mat4f(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) :
		_00(m00), _01(m01), _02(m02), _03(m03),
		_10(m10), _11(m11), _12(m12), _13(m13),
		_20(m20), _21(m21), _22(m22), _23(m23),
		_30(m30), _31(m31), _32(m32), _33(m33) {}

	explicit mat4f(const vec3f& r0, const vec3f& r1, const vec3f& r2) :
		_00(r0.x), _01(r0.y), _02(r0.z), _03(0.0f),
		_10(r1.x), _11(r1.y), _12(r1.z), _13(0.0f),
		_20(r2.x), _21(r2.y), _22(r2.z), _23(0.0f),
		_30(0.0f), _31(0.0f), _32(0.0f), _33(1.0f) {}

	explicit mat4f(const vec4f& r0, const vec4f& r1, const vec4f& r2, const vec4f& r3) :
		_00(r0.x), _01(r0.y), _02(r0.z), _03(r0.w),
		_10(r1.x), _11(r1.y), _12(r1.z), _13(r1.w),
		_20(r2.x), _21(r2.y), _22(r2.z), _23(r2.w),
		_30(r3.x), _31(r3.y), _32(r3.z), _33(r3.w) {}

	mat4f(const mat4f& m) { std::memcpy(this, &m, sizeof(float) * 16); }

	mat4f& operator=(const mat4f& m) { std::memcpy(this, &m, sizeof(float) * 16); return *this; }
	mat4f& operator+=(const mat4f& m) {
		_00 += m._00; _01 += m._01; _02 += m._02; _03 += m._03;
		_10 += m._10; _11 += m._11; _12 += m._12; _13 += m._13;
		_20 += m._20; _21 += m._21; _22 += m._22; _23 += m._23;
		_30 += m._30; _31 += m._31; _32 += m._32; _33 += m._33;
		return *this;
	}
	mat4f& operator-=(const mat4f& m) {
		_00 -= m._00; _01 -= m._01; _02 -= m._02; _03 -= m._03;
		_10 -= m._10; _11 -= m._11; _12 -= m._12; _13 -= m._13;
		_20 -= m._20; _21 -= m._21; _22 -= m._22; _23 -= m._23;
		_30 -= m._30; _31 -= m._31; _32 -= m._32; _33 -= m._33;
		return *this;
	}
	mat4f& operator*=(const mat4f& m) {
		_00 = _00*m._00 + _01*m._10 + _02*m._20 + _03*m._30;
		_01 = _00*m._01 + _01*m._11 + _02*m._21 + _03*m._31;
		_02 = _00*m._02 + _01*m._12 + _02*m._22 + _03*m._32;
		_03 = _00*m._03 + _01*m._13 + _02*m._23 + _03*m._33;

		_10 = _10*m._00 + _11*m._10 + _12*m._20 + _13*m._30;
		_11 = _10*m._01 + _11*m._11 + _12*m._21 + _13*m._31;
		_12 = _10*m._02 + _11*m._12 + _12*m._22 + _13*m._32;
		_13 = _10*m._03 + _11*m._13 + _12*m._23 + _13*m._33;

		_20 = _20*m._00 + _21*m._10 + _22*m._20 + _23*m._30;
		_21 = _20*m._01 + _21*m._11 + _22*m._21 + _23*m._31;
		_22 = _20*m._02 + _21*m._12 + _22*m._22 + _23*m._32;
		_23 = _20*m._03 + _21*m._13 + _22*m._23 + _23*m._33;

		_30 = _30*m._00 + _31*m._10 + _32*m._20 + _33*m._30;
		_31 = _30*m._01 + _31*m._11 + _32*m._21 + _33*m._31;
		_32 = _30*m._02 + _31*m._12 + _32*m._22 + _33*m._32;
		_33 = _30*m._03 + _31*m._13 + _32*m._23 + _33*m._33;
		return *this;
	}
	mat4f& operator*=(const float c) {
		_00 *= c; _01 *= c; _02 *= c; _03 *= c;
		_10 *= c; _11 *= c; _12 *= c; _13 *= c;
		_20 *= c; _21 *= c; _22 *= c; _23 *= c;
		_30 *= c; _31 *= c; _32 *= c; _33 *= c;
		return *this;
	}
	mat4f& operator/=(const float c) {
		_00 /= c; _01 /= c; _02 /= c; _03 /= c;
		_10 /= c; _11 /= c; _12 /= c; _13 /= c;
		_20 /= c; _21 /= c; _22 /= c; _23 /= c;
		_30 /= c; _31 /= c; _32 /= c; _33 /= c;
		return *this;
	}

	mat4f operator+(const mat4f& m) {
		return mat4f(
			_00 + m._00, _01 + m._01, _02 + m._02, _03 + m._03,
			_10 + m._10, _11 + m._11, _12 + m._12, _13 + m._13,
			_20 + m._20, _21 + m._21, _22 + m._22, _23 + m._23,
			_30 + m._30, _31 + m._31, _32 + m._32, _33 + m._33
		);
	}
	mat4f operator-(const mat4f& m) {
		return mat4f(
			_00 - m._00, _01 - m._01, _02 - m._02, _03 - m._03,
			_10 - m._10, _11 - m._11, _12 - m._12, _13 - m._13,
			_20 - m._20, _21 - m._21, _22 - m._22, _23 - m._23,
			_30 - m._30, _31 - m._31, _32 - m._32, _33 - m._33
		);
	}
	mat4f operator*(const mat4f& m) {
		mat4f result;

		result._00 = _00*m._00 + _01*m._10 + _02*m._20 + _03*m._30;
		result._01 = _00*m._01 + _01*m._11 + _02*m._21 + _03*m._31;
		result._02 = _00*m._02 + _01*m._12 + _02*m._22 + _03*m._32;
		result._03 = _00*m._03 + _01*m._13 + _02*m._23 + _03*m._33;

		result._10 = _10*m._00 + _11*m._10 + _12*m._20 + _13*m._30;
		result._11 = _10*m._01 + _11*m._11 + _12*m._21 + _13*m._31;
		result._12 = _10*m._02 + _11*m._12 + _12*m._22 + _13*m._32;
		result._13 = _10*m._03 + _11*m._13 + _12*m._23 + _13*m._33;

		result._20 = _20*m._00 + _21*m._10 + _22*m._20 + _23*m._30;
		result._21 = _20*m._01 + _21*m._11 + _22*m._21 + _23*m._31;
		result._22 = _20*m._02 + _21*m._12 + _22*m._22 + _23*m._32;
		result._23 = _20*m._03 + _21*m._13 + _22*m._23 + _23*m._33;

		result._30 = _30*m._00 + _31*m._10 + _32*m._20 + _33*m._30;
		result._31 = _30*m._01 + _31*m._11 + _32*m._21 + _33*m._31;
		result._32 = _30*m._02 + _31*m._12 + _32*m._22 + _33*m._32;
		result._33 = _30*m._03 + _31*m._13 + _32*m._23 + _33*m._33;

		return result;
	}
	mat4f operator*(const float c) {
		return mat4f(
			_00*c, _01*c, _02*c, _03*c,
			_10*c, _11*c, _12*c, _13*c,
			_20*c, _21*c, _22*c, _23*c,
			_30*c, _31*c, _32*c, _33*c
		);
	}
	mat4f operator/(const float c) {
		return mat4f(
			_00/c, _01/c, _02/c, _03/c,
			_10/c, _11/c, _12/c, _13/c,
			_20/c, _21/c, _22/c, _23/c,
			_30/c, _31/c, _32/c, _33/c
		);
	}

	static const mat4f Identity;
	static const mat4f Zero;
	static const mat4f One;
};

// ------- CONSTANTS -------------

extern const float EPSILON;
extern const float PI;
extern const float TWOPI;
extern const float PIDIV2;
extern const float PIDIV4;

// ------- FUNCTIONS -------------

// ------- LENGTH FUNCTIONS --------------

inline float	LengthSqr(const vec2f& v) { return v.x*v.x + v.y*v.y; }
inline float	LengthSqr(const vec3f& v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
inline float	LengthSqr(const vec4f& v) { return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }
inline float	LengthSqr(const quaternion& v) { return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }

template<typename Vec>
inline float	Length(const Vec& v) { return sqrtf(LengthSqr(v)); }

template<typename Vec>
Vec				Normalize(const Vec& v) {
	return (v / Length(v));
}

template<typename Vec>
inline float	DistanceSqr(const Vec& v1, const Vec& v2) { Vec diff = v1 - v2; return LengthSqr(diff); }

template<typename Vec>
inline float	Distance(const Vec& v1, const Vec& v2) { return sqrtf(DistanceSqr(v1, v2)); }

// --------- MATRIX FUNCTIONS -------------

mat4f Transpose(const mat4f& m);
mat4f Inverse(const mat4f& in);

mat4f Translation(const vec3f& translation);
mat4f Translation(const float xt, const float yt, const float zt);

mat4f Scale(const float scale);
mat4f Scale(const vec3f& scale);
mat4f Scale(const float xs, const float ys, const float zs);

mat4f RotationX(const float rad);
mat4f RotationY(const float rad);
mat4f RotationZ(const float rad);

mat4f RotationMatrixFromEulerAngles(const vec3f& euler);
mat4f RotationMatrixFromEulerAngles(const float xr, const float yr, const float zr);
mat4f AxisAngleToRotationMatrix(const vec3f& axis, const float angle);

mat4f PerspectiveFov(const float fov, const float aspect_ratio, const float near_z, const float far_z);
mat4f Orthographic(const float width, const float height, const float near_z, const float far_z);

// --------- QUATERNION FUNCTIONS ---------

inline quaternion	Conjugate(const quaternion& q) {
	return quaternion(
		-q.x, -q.y, -q.z, q.w
	);
}
inline quaternion	Inverse(const quaternion& q) {
	return Conjugate(q) / LengthSqr(q);
}

inline vec4f		QuaternionToAxisAngle(const quaternion& q) {
	const float angle = 2.0f*acosf(q.w);
	const vec3f axis = vec3f(q.x, q.y, q.z) / (sqrt(1 - q.w*q.w));
	return vec4f(axis, angle);
}
mat4f				QuaternionToRotationMatrix(const quaternion& q);

quaternion			RotationMatrixToQuaternion(const mat4f& pm);
inline quaternion	AxisAngleToQuaternion(const vec3f& axis, const float angle) {
	const float w = cosf(angle / 2);
	return quaternion(axis * sinf(angle / 2), w);
}
inline quaternion	AxisAngleToQuaternion(const vec4f& axis_angle) {
	return AxisAngleToQuaternion(axis_angle.xyz(), axis_angle.w);
}

// ------ OPERATORS ---------------

template<typename Vec>
inline Vec		operator*(const float c, const Vec& v) {
	return v * c;
}
inline vec4f	operator*(const vec4f& v, const mat4f& m) {
	return vec4f(
		v.x*m._00 + v.y*m._10 + v.z*m._20 + v.w*m._30,
		v.x*m._01 + v.y*m._11 + v.z*m._21 + v.w*m._31,
		v.x*m._02 + v.y*m._12 + v.z*m._22 + v.w*m._32,
		v.x*m._03 + v.y*m._13 + v.z*m._23 + v.w*m._33
	);
}

// ------ ANGLE FUNCTIONS -------------

inline constexpr float	Rad(const float angles) {
	return angles * (PI / 180.0f);
}
inline constexpr float	Angle(const float rad) {
	return rad * (180.0f / PI);
}
inline float			DeltaAngle(const float a1, const float a2) {
	static auto cmod = [](const float x, const float y) {
		return x - floorf(x / y) * y;
	};
	float a = a2 - a1;
	a = cmod(a + 180, 360) - 180;
	return a;
}
inline bool				IsAngleEqual(const float r1, const float r2, const float eps = EPSILON) {
	return abs(DeltaAngle(r1, r2)) <= eps;
}
inline bool				IsRadianEqual(const float r1, const float r2, const float eps = EPSILON) {
	return IsAngleEqual(Angle(r1), Angle(r2), eps);
}


// ------- EQUALITY TESTING ----------

bool IsEqual(const vec2f& v1, const vec2f& v2, const float eps = EPSILON);
bool IsEqual(const vec3f& v1, const vec3f& v2, const float eps = EPSILON);
bool IsEqual(const vec4f& v1, const vec4f& v2, const float eps = EPSILON);
bool IsEqual(const quaternion& v1, const quaternion& v2, const float eps = EPSILON);
bool IsEqual(const mat4f& v1, const mat4f& v2, const float eps = EPSILON);
bool IsEqual(const mat4f& v1, const DirectX::XMMATRIX& v2, const float eps = EPSILON);

template<typename V1, typename V2>
bool operator==(const V1& lhs, const V2& rhs) {
	return IsEqual(lhs, rhs, EPSILON);
}

template<typename V1, typename V2>
bool operator!=(const V1& lhs, const V2& rhs) {
	return !IsEqual(lhs, rhs, EPSILON);
}



// -------- VECTOR FUNCTIONS -----------------

inline float	Dot(const vec2f& u, const vec2f& v) {
	return u.x*v.x + u.y*v.y;
}
inline float	Dot(const vec3f& u, const vec3f& v) {
	return u.x*v.x + u.y*v.y + u.z*v.z;
}
inline float	Dot(const vec4f& u, const vec4f& v) {
	return u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w;
}

vec3f			Cross(const vec3f& u, const vec3f& v);

template<typename Vec>
inline float	AngleBetween(const Vec& u, const Vec& v) {
	return acosf(Dot(u, v)) * (1 / (Length(u)*Length(v)));
}

// ---------- UTILITY FUNCTIONS ----------------

float			Clamp(const float u, const float min, const float max);
vec2f			Clamp(const vec2f& u, const vec2f& min, const vec2f& max);
vec3f			Clamp(const vec3f& u, const vec3f& min, const vec3f& max);
vec4f			Clamp(const vec4f& u, const vec4f& min, const vec4f& max);

inline float	Min(const float u, const float v) { return (u <= v) ? u : v; }
vec2f			Min(const vec2f& u, const vec2f& v);
vec3f			Min(const vec3f& u, const vec3f& v);
vec4f			Min(const vec4f& u, const vec4f& v);

inline float	Max(const float u, const float v) { return (u >= v) ? u : v; }
vec2f			Max(const vec2f& u, const vec2f& v);
vec3f			Max(const vec3f& u, const vec3f& v);
vec4f			Max(const vec4f& u, const vec4f& v);

template<typename Vec>
Vec				Lerp(const Vec& v1, const Vec& v2, float t) {
	return ((1.0f - t)*v1) + (t*v2);
}

template<typename Vec>
Vec				SmoothStep(const Vec& edge0, const Vec& edge1, const Vec& v) {
	Vec t = Clamp((v - edge0) / (edge1 - edge0), Vec::Zero, Vec::One);
	return t*t*((3.0f*Vec::One) - (2.0f*t));
}

template<>
float			SmoothStep(const float& edge0, const float& edge1, const float& v);

// ----------- TRANSFORMATIONS ---------------

inline vec3f	RotatePoint(const vec3f& v, const quaternion& q) {
	quaternion p = Conjugate(q) * (quaternion(v, 0.0f) * q);
	return vec3f(p.x, p.y, p.z);
}

inline vec4f	RotatePoint(const vec4f& v, const quaternion& q) {
	auto p = RotatePoint(v.xyz(), q);
	return vec4f(p, 1.0f);
}
inline vec4f	RotateVector(const vec4f& v, const quaternion& q) {
	auto p = RotatePoint(v.xyz(), q);
	return vec4f(p, 0.0f);
}

inline vec4f	RotatePoint(const vec4f& v, const vec3f& axis, const float angle) {
	return RotatePoint(v, AxisAngleToQuaternion(axis, angle));
}
inline vec4f	RotatePoint(const vec4f& v, const vec4f& axis_angle) {
	return RotatePoint(v, AxisAngleToQuaternion(axis_angle));
}
inline vec4f	RotateVector(const vec4f& v, const vec3f& axis, const float angle) {
	return RotateVector(v, AxisAngleToQuaternion(axis, angle));
}
inline vec4f	RotateVector(const vec4f& v, const vec4f& axis_angle) {
	return RotateVector(v, AxisAngleToQuaternion(axis_angle));
}

vec3f AxisAngleToEuler(const vec3f& axis, float angle);

} // namespace pn