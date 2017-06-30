#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

#include <string>

#include <DirectXMath.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

namespace MathUnitTest {
TEST_CLASS(Mat4fTest) {
public:
	TEST_METHOD(SizeTest) {
		Assert::AreEqual(static_cast<unsigned int>(64), sizeof(mat4f));
	}

	TEST_METHOD(ConstructorTest) {
		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
				5.0f, 6.0f, 7.0f, 8.0f,
				9.0f, 10.0f, -11.0f, 12.0f,
				13.0f, -14.0f, 15.0f, 16.0f);

		mat4f m2(m);

		Assert::IsTrue(m == m2);

		mat4f m3 = m;
		Assert::IsTrue(m == m3);

		mat4f m4;
		Assert::IsTrue(m4 == mat4f::Identity);
	}

	TEST_METHOD(AssignmentOp) {
		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
				5.0f, 6.0f, 7.0f, 8.0f,
				9.0f, 10.0f, -11.0f, 12.0f,
				13.0f, -14.0f, 15.0f, 16.0f);

		mat4f m2;
		m2 = m;
		Assert::IsTrue(m2 == m);
	}

	TEST_METHOD(MatrixMultTest) {
		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
				5.0f, 6.0f, 7.0f, 8.0f,
				9.0f, 10.0f, -11.0f, 12.0f,
				13.0f, -14.0f, 15.0f, 16.0f);

		mat4f m2(
			2.0f, 1.0f, 3.0f, 1.0f,
			5.0f, -2.0f, 6.0f, 10.0f,
			-12.0f, 13.0f, 4.0f, 5,
			1, -1, 2, 9
		);

		mat4f result(
			-28, 40, 19, 0,
			-36, 76, 95, 172,
			212, -166, 67, 162,
			-208, 220, 47, 92
		);

		Assert::IsTrue(m*mat4f::Identity == m);
		Assert::IsTrue(m*mat4f::Zero == mat4f::Zero);
		Assert::IsTrue((m * m2) == result);
	}

	TEST_METHOD(InverseTest) {
		Assert::IsTrue(Inverse(mat4f::Identity) == mat4f::Identity);

		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
			  5.0f, 6.0f, 7.0f, 8.0f,
			  9.0f, 10.0f, -11.0f, 12.0f,
			  13.0f, -14.0f, 15.0f, 16.0f);

		auto i = Inverse(m);
		Assert::IsTrue(m * i == mat4f::Identity);
		Assert::IsTrue(i * m == mat4f::Identity);
		auto j = Inverse(i);
		Assert::IsTrue(j == m);
	}

	TEST_METHOD(TransposeTest) {
		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
				5.0f, 6.0f, 7.0f, 8.0f,
				9.0f, 10.0f, -11.0f, 12.0f,
				13.0f, -14.0f, 15.0f, 16.0f);

		mat4f t(1.0f, 5.0f, 9.0f, 13.0f,
				2.0f, 6.0f, 10.0f, -14.0f,
				3.0f, 7.0f, -11.0f, 15.0f,
				-4.0f, 8.0f, 12.0f, 16.0f);

		Assert::IsTrue(t == Transpose(m));
	}

	TEST_METHOD(TranslationTest) {
		vec4f v(1.0f, 2.0f, 3.0f, 1.0f);
		Assert::IsTrue(v * Translation(3.0f, -1.0f, 3.0f) == vec4f(4.0f, 1.0f, 6.0f, 1.0f));
	}

	TEST_METHOD(ScaleTest) {
		vec4f v(1.0f, 2.0f, 3.0f, 1.0f);
		Assert::IsTrue(v * Scale(2.0f, 3.0f, 5.0f) == vec4f(2.0f, 6.0f, 15.0f, 1.0f));
	}

	TEST_METHOD(AxisRotationTest) { 
		{
			vec4f v(1.0f, 0.0f, 0.0f, 1.0f);

			Assert::IsTrue(v * RotationX(PIDIV2) == v);
			Assert::IsTrue(v * RotationY(PIDIV2) == vec4f(0.0f, 0.0f, -1.0f, 1.0f));
			Assert::IsTrue(v * RotationZ(PIDIV2) == vec4f(0.0f, 1.0f, 0.0f, 1.0f));		
		}

		{
			vec4f v(-1.0f, 0.0f, 0.0f, 1.0f);
			Assert::IsTrue(v * RotationX(PIDIV2) == v);
			Assert::IsTrue(v * RotationY(PIDIV2) == vec4f(0.0f, 0.0f, 1.0f, 1.0f));
			Assert::IsTrue(v * RotationZ(PIDIV2) == vec4f(0.0f, -1.0f, 0.0f, 1.0f));
		}

		{
			vec4f v(0.0f, 1.0f, 0.0f, 1.0f);
			Assert::IsTrue(v * RotationX(PIDIV2) == vec4f(0.0f, 0.0f, 1.0f, 1.0f));
			Assert::IsTrue(v * RotationY(PIDIV2) == v);
			Assert::IsTrue(v * RotationZ(PIDIV2) == vec4f(-1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

	TEST_METHOD(EulerRotationTest) {

		{
			mat4f m = EulerToRotationMatrix(0.0f, 0.0f, 0.0f);
			Assert::IsTrue(m == mat4f::Identity);
		}

		for (float t = -TWOPI; t <= TWOPI; t += 0.1f) {
			mat4f m = EulerToRotationMatrix(t, 0.0f, 0.0f);
			auto xm = RotationX(t);
			auto diff = m - xm;
			Assert::IsTrue(IsEqual(diff, mat4f::Zero));
		}

		for (float t = -TWOPI; t <= TWOPI; t += 0.1f) {
			mat4f m = EulerToRotationMatrix(0, t, 0.0f);
			auto xm = RotationY(t);
			auto diff = m - xm;
			Assert::IsTrue(IsEqual(diff, mat4f::Zero));
		}

		for (float t = -TWOPI; t <= TWOPI; t += 0.1f) {
			mat4f m = EulerToRotationMatrix(0, 0, t);
			auto xm = RotationZ(t);
			auto diff = m - xm;
			Assert::IsTrue(IsEqual(diff, mat4f::Zero));
		}

/*#ifdef NDEBUG
		for (float t = -TWOPI; t <= TWOPI; t += 0.1f) {
			for (float u = -TWOPI; u <= TWOPI; u += 0.1f) {
				for (float v = -TWOPI; v <= TWOPI; v += 0.1f) {
					mat4f m = EulerToRotationMatrix(t, u, v);
					auto xm = RotationZ(v) * RotationX(t) * RotationY(u);
					Assert::IsTrue(m == xm);
				}
			}
		}
#else
		for (float t = -TWOPI; t <= TWOPI; t += 0.5f) {
			for (float u = -TWOPI; u <= TWOPI; u += 0.5f) {
				for (float v = -TWOPI; v <= TWOPI; v += 0.5f) {
					mat4f m = EulerToRotationMatrix(t, u, v);
					//auto xm = RotationZ(v) * RotationX(t) * RotationY(u);
					auto dxm = DirectX::XMMatrixRotationRollPitchYaw(t, u, v);
					if (m != dxm) {
						Assert::IsTrue(true);
					}
					Assert::IsTrue(m == dxm);
				}
			}
		}
#endif*/
	}

	TEST_METHOD(PerspectiveFovTest) {
		mat4f m = PerspectiveFov(Rad(60.0f), 1.3f, 0.01f, 1000.0f);
		auto m2 = DirectX::XMMatrixPerspectiveFovLH(Rad(60.0f), 1.3f, 0.01f, 1000.0f);
		Assert::IsTrue(m == m2);

/*#ifdef NDEBUG
		for (float t = 0.1f; t <= TWOPI; t += 0.1f) {
			for (float ar = 0.5f; ar <= 2.5f; ar += 0.1f) {
				for (float n = 0.00001f; n <= 1.0f; n += 0.00001f) {
					mat4f m = PerspectiveFov(t, ar, n, 1000.0f);
					auto xm = DirectX::XMMatrixPerspectiveFovLH(t, ar, n, 1000.0f);
					Assert::IsTrue(IsEqual(m, xm, 0.001f));
				}
			}
		}
#endif */
	}

	TEST_METHOD(OrthographicTest) {
		const float w = 1231.0f, h = 642.0f, n = 0.0001f, f = 1000.0f;
		mat4f m = Orthographic(w, h, n, f);
		auto m2 = DirectX::XMMatrixOrthographicLH(w, h, n, f);
		Assert::IsTrue(m == m2);
	}

	TEST_METHOD(LookAtTest) {
		{
			vec3f p(1.0f, -3.0f, 5.0f);
			vec3f f(0.0f, 0.3f, 0.3f);
			vec3f u(0.0f, 1.0f, 0.0f);
			mat4f m = LookAt(p, f, u);
			auto m2 = DirectX::XMMatrixLookAtLH({ p.x, p.y, p.z }, { f.x, f.y, f.z }, { u.x, u.y, u.z });
			Assert::IsTrue(m == m2);
		}

		{
			vec3f p(1.0f, -3.0f, 5.0f);
			vec3f u(0.0f, 1.0f, 0.0f);
			vec3f f = u;
			mat4f m = LookAt(p, f, u);
			auto m2 = DirectX::XMMatrixLookAtLH({ p.x, p.y, p.z }, { f.x, f.y, f.z }, { u.x, u.y, u.z });
			Assert::IsTrue(m == m2);
		}
	}

	TEST_METHOD(ToCoordinateSystemTest) {
		vec3f p(1.0f, -3.0f, 5.0f);
		vec3f f(0.0f, 0.3f, 0.3f);
		vec3f u(0.0f, 1.0f, 0.0f);
		mat4f m = ToCoordinateSystem(p, f, u);
		
		vec4f pos(1.0f, -3.0f, 5.0f, 1.0f);
		auto r = pos * m;
		Assert::IsTrue(r.xyz() == vec3f::Zero);

		vec4f x = vec4f::UnitX;
	}

	/*TEST_METHOD(DecomposeTest) {
		vec3f t(90.0f, -102.4f, 12.0f);
		vec3f r(Rad(90.0f), Rad(0.0f), Rad(0.0f));
		vec3f s(1.0f, 1.0f, 1.0f);
		
		mat4f m = Scale(s) * RotationMatrixFromEulerAngles(r) * Translation(t);

		auto tra = GetTranslation(m);
		Assert::IsTrue(t == tra);

		auto sca = GetScale(m);
		Assert::IsTrue(IsEqual(s, sca));
		
		auto rot = GetRotation(m);
		Assert::IsTrue(IsRadianEqual(r.x, rot.x) && IsRadianEqual(r.y, rot.y) && IsRadianEqual(r.z, rot.z));

	} */

	TEST_METHOD(SRTMatrixTest) {
		auto test = [](const vec3f& s, const vec3f& r, const vec3f& t) {
			auto m = SRTMatrix(s, r, t);
			auto dxm = DirectX::XMMatrixScaling(s.x, s.y, s.z) * 
				DirectX::XMMatrixRotationRollPitchYaw(r.x, r.y, r.z) * 
				DirectX::XMMatrixTranslation(t.x, t.y, t.z);

			Assert::IsTrue(m == dxm);
		};

		{
			vec3f t(90.0f, -102.4f, 12.0f);
			vec3f r(Rad(90.0f), Rad(0.0f), Rad(0.0f));
			vec3f s(1.0f, 1.0f, 1.0f);
			test(s, r, t);
		}

		{
			vec3f t(92130.0f, -1032.4f, -112.0f);
			vec3f r(Rad(91110.0f), Rad(-42.1241f), Rad(312.0f));
			vec3f s(2.0f, -1.0f, -121.0f);
			test(s, r, t);
		}
	}
	
};
}