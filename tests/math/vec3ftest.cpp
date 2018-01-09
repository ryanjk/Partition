#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

namespace MathUnitTest {
TEST_CLASS(Vec3fTest) {
public:
	TEST_METHOD(SizeTest) {
		Assert::AreEqual(static_cast<size_t>(12), sizeof(vec3f));
	}

	TEST_METHOD(CrossProductTest) {
		vec3f u(3.0f, 1.0f, -1.0f);
		vec3f v(2.0f, 3.0f, 0.0f);

		Assert::IsTrue(vec3f(3, -2, 7) == Cross(u, v));
		Assert::IsTrue(vec3f::Zero == Cross(u, u));
	}
};
}