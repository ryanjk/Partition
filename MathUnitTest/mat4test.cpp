#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

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

		Assert::IsTrue(m*mat4f::Identity == m);
		Assert::IsTrue(m*mat4f::Zero == mat4f::Zero);
	}

	TEST_METHOD(InverseTest) {
		mat4f m(1.0f, 2.0f, 3.0f, -4.0f,
			  5.0f, 6.0f, 7.0f, 8.0f,
			  9.0f, 10.0f, -11.0f, 12.0f,
			  13.0f, -14.0f, 15.0f, 16.0f);

		Assert::IsTrue(Inverse(mat4f::Identity) == mat4f::Identity);
		auto i = Inverse(m);
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

	}
	
};
}