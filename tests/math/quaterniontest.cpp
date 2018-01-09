#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

namespace MathUnitTest {
TEST_CLASS(QuaternionTest) {
public:
	TEST_METHOD(SizeTest) {
		Assert::AreEqual(static_cast<size_t>(16), sizeof(quaternion));
	}

	TEST_METHOD(InverseTest) {
		quaternion q(0.003f, 0.931f, -0.310f, 0.193f);
		auto inv = Inverse(q);

		Assert::IsTrue(q*inv == quaternion::Identity);
	}

	TEST_METHOD(AxisAngleTest) {
		quaternion q(0.003f, 0.931f, -0.310f, 0.193f);
		q = Normalize(q);

		auto axis_angle = QuaternionToAxisAngle(q);
		Assert::IsTrue(axis_angle == vec4f(0.003057f, 0.94878f, -0.315920, 2.75317f));
		Assert::IsTrue(AxisAngleToQuaternion(axis_angle) == q);
	}

	TEST_METHOD(RotateVectorTest) {
		quaternion q2 = AxisAngleToQuaternion(vec3f(1.0f, 0.0f, 0.0f), Rad(90.0f));
		vec3f v2(0.0f, 1.0f, 0.0f);
		auto r2 = RotatePoint(v2,q2);
		Assert::IsTrue(r2 == vec3f(0.0f, 0.0f, 1.0f));
	}

	TEST_METHOD(QuaternionToEulerTest) {
		for (float x = -TWOPI; x <= TWOPI; x += 0.1f) {
			for (float y = -TWOPI; y <= TWOPI; y += 0.1f) {
				for (float z = -TWOPI; z <= TWOPI; z += 0.1f) {
					vec3f v(x,y,z);
					auto q = EulerToQuaternion(v);
					auto e = QuaternionToEuler(q);
					Assert::IsTrue(IsRadianEqual(v.x, e.x, 0.01f));
					Assert::IsTrue(IsRadianEqual(v.y, e.y, 0.01f));
					Assert::IsTrue(IsRadianEqual(v.z, e.z, 0.01f));
				}
			}
		}
		/*vec3f v(2.0f, -1.32f, 5.82f);
		auto q = EulerToQuaternion(v);
		auto e = QuaternionToEuler(q);
		Assert::IsTrue(IsRadianEqual(v.x, e.x, 0.0001f));
		Assert::IsTrue(IsRadianEqual(v.y, e.y, 0.0001f));
		Assert::IsTrue(IsRadianEqual(v.z, e.z, 0.0001f)); */
	}

	TEST_METHOD(QuaternionToRotationMatrixTest) {

		{
			const float t = PIDIV2;
			quaternion q(AxisAngleToQuaternion(vec4f(1.0f, 0.0f, 0.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationX(t));
		}

		{
			const float t = PIDIV2;
			quaternion q(AxisAngleToQuaternion(vec4f(0.0f, 1.0f, 0.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationY(t));
		}

		{
			const float t = PIDIV2;
			quaternion q(AxisAngleToQuaternion(vec4f(0.0f, 0.0f, 1.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationZ(t));
		}

		{
			const float t = 0.13f;
			quaternion q(AxisAngleToQuaternion(vec4f(1.0f, 0.0f, 0.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationX(t));
		}

		{
			const float t = -1.1f;
			quaternion q(AxisAngleToQuaternion(vec4f(0.0f, 1.0f, 0.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationY(t));
		}

		{
			const float t = 3.9f;
			quaternion q(AxisAngleToQuaternion(vec4f(0.0f, 0.0f, 1.0f, t)));
			q = Normalize(q);
			mat4f m = QuaternionToRotationMatrix(q);
			Assert::IsTrue(m == RotationZ(t));
		}
	}
};
}