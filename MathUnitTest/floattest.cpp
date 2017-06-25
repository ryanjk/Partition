#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

static const float EPS = 0.00001f;

namespace MathUnitTest {
TEST_CLASS(FloatTest) {
public:
	
	TEST_METHOD(SmoothStepTest) {
		float edge0 = 4.0f;
		float edge1 = 11.0f;
		
		{
			float x = 1.0f;
			Assert::AreEqual(0.0f, SmoothStep(edge0, edge1, x));
		}

		{
			float x = 40.0f;
			Assert::AreEqual(1.0f, SmoothStep(edge0, edge1, x));
		}

		{
			float x = 6.0f;
			Assert::AreEqual(0.198251f, SmoothStep(edge0, edge1, x), EPS);
		}
	}

	TEST_METHOD(ClampTest) {
		float min = 1.0f;
		float max = 40.0f;

		{
			float x = -10.0f;
			Assert::AreEqual(min, Clamp(x, min, max));
		}

		{
			float x = 30.0f;
			Assert::AreEqual(x, Clamp(x, min, max));
		}

		{
			float x = 100.0f;
			Assert::AreEqual(max, Clamp(x, min, max));
		}
	}

	TEST_METHOD(LerpTest) {
		float x1 = 0.0f;
		float x2 = 10.0f;

		Assert::AreEqual(x1, Lerp(x1, x2, 0.0f), EPS);
		Assert::AreEqual(x2, Lerp(x1, x2, 1.0f), EPS);
		Assert::AreEqual(5.0f, Lerp(x1, x2, 0.5f), EPS);
	}

	TEST_METHOD(AngleEqualTest) {
		Assert::IsTrue(IsAngleEqual(0.0f, 0.0f));
		Assert::IsTrue(IsAngleEqual(0.0f, 360.0f));
		Assert::IsTrue(IsAngleEqual(0.0f, -360.0f));
		
		Assert::IsTrue(IsAngleEqual(180.0f, 180.0f));
		Assert::IsTrue(IsAngleEqual(180.0f, 180.0f + 360.0f));
		Assert::IsTrue(IsAngleEqual(180.0f, 180.0f - 360.0f));

		Assert::IsTrue(IsAngleEqual(10.0f, 10.0f));
		Assert::IsTrue(IsAngleEqual(-10.0f, 350.0f));
		Assert::IsTrue(IsAngleEqual(20.0f, 380.0f));

		Assert::IsFalse(IsAngleEqual(0.0f, 1.0f));
		Assert::IsFalse(IsAngleEqual(0.0f, -1.0f));
	}

	TEST_METHOD(RadianEqualTest) {
		Assert::IsTrue(IsRadianEqual(0.0f, 0.0f));
		Assert::IsTrue(IsRadianEqual(0.0f, TWOPI));
		Assert::IsTrue(IsRadianEqual(0.0f, -TWOPI));

		Assert::IsFalse(IsRadianEqual(Rad(0.0f), Rad(1.0f)));
		Assert::IsFalse(IsRadianEqual(Rad(0.0f), Rad(-1.0f)));

		Assert::IsTrue(IsRadianEqual(PIDIV2, PIDIV2));
		Assert::IsTrue(IsRadianEqual(PIDIV2, PIDIV2 + TWOPI));
		Assert::IsTrue(IsRadianEqual(PIDIV2, PIDIV2 - TWOPI));

		Assert::IsTrue(IsRadianEqual(Rad(10.0f), Rad(10.0f)));
		Assert::IsTrue(IsRadianEqual(Rad(-10.0f), Rad(350.0f)));
		Assert::IsTrue(IsRadianEqual(Rad(20.0f), Rad(380.0f)));

	}
};
}