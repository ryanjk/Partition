#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

namespace MathUnitTest {
TEST_CLASS(QuaternionTest) {
public:
	TEST_METHOD(SizeTest) {
		Assert::AreEqual(static_cast<unsigned int>(16), sizeof(quaternion));
	}

	TEST_METHOD(InverseTest) {
		quaternion q(0.003f, 0.931f, -0.310f, 0.193f);
		auto inv = Inverse(q);

		Assert::IsTrue(q*inv == quaternion::Identity);
	}

	TEST_METHOD(AxisAngleTest) {
		quaternion q(0.003f, 0.931f, -0.310f, 0.193f);
		auto axis_angle = QuaternionToAxisAngle(q);
		Assert::IsTrue(axis_angle == vec4f(0.003057f, 0.948839f, -0.315940, 2.75315f));

	}

	TEST_METHOD(RotateVectorTest) {
		quaternion q2 = AxisAngleToQuaternion(vec3f(1.0f, 0.0f, 0.0f), Rad(90.0f));
		vec3f v2(0.0f, 1.0f, 0.0f);
		auto r2 = RotatePoint(v2,q2);
		Assert::IsTrue(r2 == vec3f(0.0f, 0.0f, 1.0f));
	}

	TEST_METHOD(RotationMatrixTest) {
		quaternion q(0.003f, 0.931f, -0.310f, 0.193f);
		q = Normalize(q);
		Assert::AreEqual(1.0f, Length(q), 0.0001f);

		auto to_mat = QuaternionToRotationMatrix(q);
		auto to_q = RotationMatrixToQuaternion(to_mat);
		Assert::IsTrue(to_q == q);
	}
};
}