#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Math.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace pn;

namespace MathUnitTest
{		
	TEST_CLASS(Vec2fTest)
	{
	public:
		
		TEST_METHOD(SizeTest) {
			Assert::AreEqual(static_cast<unsigned int>(8), sizeof(vec2f));
		}

		TEST_METHOD(ConstructorTest) {
			{
				vec2f v;
				Assert::AreEqual(0.0f, v.x);
				Assert::AreEqual(0.0f, v.y);
			}

			{
				vec2f v{};
				Assert::AreEqual(0.0f, v.x);
				Assert::AreEqual(0.0f, v.y);
			}

			{
				vec2f v(1.0f, -3.0f);
				Assert::AreEqual(1.0f, v.x);
				Assert::AreEqual(-3.0f, v.y);

				{
					vec2f v2(v);
					Assert::AreEqual(1.0f, v2.x);
					Assert::AreEqual(-3.0f, v2.y);
				}

				{
					vec2f v2 = v;
					Assert::AreEqual(1.0f, v2.x);
					Assert::AreEqual(-3.0f, v2.y);
				}
			}

			{
				vec2f v(2.0, 4.0);
				Assert::AreEqual(2.0f, v.x);
				Assert::AreEqual(4.0f, v.y);
			}

			{
				vec2f v(2, 4);
				Assert::AreEqual(2.0f, v.x);
				Assert::AreEqual(4.0f, v.y);
			}

		}

		TEST_METHOD(VectorAdditionTest) {
			vec2f v1(3.0f, 1.0f);
			vec2f v2(-7.0f, 2.0f);
			vec2f sum = v1 + v2;
			Assert::AreEqual(-4.0f, sum.x);
			Assert::AreEqual(3.0f, sum.y);
		}

		TEST_METHOD(VectorSubtractionTest) {
			vec2f v1(3.0f, 1.0f);
			vec2f v2(-7.0f, 2.0f);

			{
				vec2f diff = v1 - v2;
				Assert::AreEqual(10.0f, diff.x);
				Assert::AreEqual(-1.0f, diff.y);
			}

			{
				vec2f diff = v2 - v1;
				Assert::AreEqual(-10.0f, diff.x);
				Assert::AreEqual(1.0f, diff.y);
			}
		}

		TEST_METHOD(VectorMultTest) {
			vec2f u(2.0f, 10.0f);
			vec2f v(10.0f, 40.0f);
			Assert::IsTrue(u*v == vec2f(20.0f, 400.0f));
			
			Assert::IsTrue(u*vec2f::Zero == vec2f::Zero);
			Assert::IsTrue(u*vec2f::One == u);
		}

		TEST_METHOD(VectorDivTest) {
			vec2f u(10.0f, 40.0f);
			vec2f v(2.0f, 10.0f);
			Assert::IsTrue(u/v == vec2f(5.0f, 4.0f));

			Assert::IsTrue(u/vec2f::One == u);
		}

		TEST_METHOD(ScalarMultTest) {
			vec2f v(6.0f, 10.0f);
			float c = 3.0f;
			
			{
				vec2f p = v * c;
				Assert::AreEqual(18.0f, p.x);
				Assert::AreEqual(30.0f, p.y);
			}

			{
				vec2f p = c * v;
				Assert::AreEqual(18.0f, p.x);
				Assert::AreEqual(30.0f, p.y);
			}
		}

		TEST_METHOD(ScalarDivTest) {
			vec2f v(6.0f, 30.0f);
			float c = 3.0f;

			{
				vec2f p = v / c;
				Assert::AreEqual(2.0f, p.x);
				Assert::AreEqual(10.0f, p.y);
			}

		}

		TEST_METHOD(EqualOperatorTest) {
			vec2f v1(1.0f, 340.0f);

			{
				vec2f v2(v1);
				Assert::IsTrue(v1 == v2);
				Assert::IsTrue(v2 == v1);
				Assert::IsFalse(v1 != v2);
				Assert::IsFalse(v2 != v1);
			}
		}
		
		TEST_METHOD(AddOpTest) {
			vec2f v(4.0f, 10.0f);
			v += vec2f(10.0f, -10.0f);
			Assert::IsTrue(v == vec2f(14.0f, 0.0f));
		}

		TEST_METHOD(SubOpTest) {
			vec2f v(4.0f, 10.0f);
			v -= vec2f(10.0f, -10.0f);
			Assert::IsTrue(v == vec2f(-6.0f, 20.0f));
		}
		
		TEST_METHOD(MultOpTest) {
			vec2f v(4.0f, 10.0f);
			v *= 2.0f;
			Assert::IsTrue(v == vec2f(8.0f, 20.0f));
		}

		TEST_METHOD(DivOpTest) {
			vec2f v(4.0f, 10.0f);
			v /= 2.0f;
			Assert::IsTrue(v == vec2f(2.0f, 5.0f));
		}

		TEST_METHOD(DivVectorOpTest) {
			vec2f v(4.0f, 10.0f);
			vec2f v2(2.0f, 5.0f);
			v /= v2;
			Assert::IsTrue(v == vec2f(2.0f, 2.0f));
		}

		TEST_METHOD(MultVectorOpTest) {
			vec2f v(4.0f, 10.0f);
			vec2f v2(2.0f, 5.0f);
			v *= v2;
			Assert::IsTrue(v == vec2f(8.0f, 50.0f));
		}

		TEST_METHOD(LengthTest) {
			{
				vec2f v(3.0f, 4.0f);
				Assert::AreEqual(25.0f, LengthSqr(v));
				Assert::AreEqual(5.0f, Length(v));
			}

			{
				vec2f v;
				Assert::AreEqual(0.0f, LengthSqr(v));
				Assert::AreEqual(0.0f, Length(v));
			}
		}

		TEST_METHOD(NormalizeTest) {
			vec2f v(4.0f, 10.0f);
			Assert::AreNotEqual(1.0f, Length(v));

			vec2f n = Normalize(v);
			Assert::AreEqual(1.0f, Length(n));
		}

		TEST_METHOD(ClassConstantsTest) {
			Assert::IsTrue(vec2f::Zero == vec2f(0.0f, 0.0f));
			Assert::IsTrue(vec2f::One == vec2f(1.0f, 1.0f));
			Assert::IsTrue(vec2f::UnitX == vec2f(1.0f, 0.0f));
			Assert::IsTrue(vec2f::UnitY == vec2f(0.0f, 1.0f));
		}

		TEST_METHOD(DistanceTest) {
			vec2f v1(7.0f, 3.0f);
			vec2f v2(10.0f, 7.0f);
			Assert::AreEqual(25.0f, DistanceSqr(v1, v2));
			Assert::AreEqual(5.0f, Distance(v1, v2));
		}

		TEST_METHOD(DotTest) {
			vec2f v1(3.0f, 2.0f);
			vec2f v2(2.0f, 2.0f);
			Assert::AreEqual(10.0f, Dot(v1, v2));
		}

		TEST_METHOD(LerpTest) {
			vec2f u(3.0f, 1.0f);
			vec2f v(5.0f, 7.0f);
			Assert::IsTrue(Lerp(u, v, 0.0f) == u);
			Assert::IsTrue(Lerp(u, v, 1.0f) == v);
			Assert::IsTrue(Lerp(u, v, 0.5f) == vec2f(4.0f, 4.0f));
		}

		TEST_METHOD(ClampTest) {
			vec2f v(3.0f, 10.0f);
			vec2f min(4.0f, 7.0f);
			vec2f max(10.0f, 9.0f);
			Assert::IsTrue(Clamp(v, min, max) == vec2f(4.0f, 9.0f));
		}

		TEST_METHOD(MinTest) {
			vec2f v(10.0f, -10.0f);
			vec2f v2(12.0f, -7.0f);
			Assert::IsTrue(Min(v, v2) == vec2f(10.0f, -10.0f));
		}

		TEST_METHOD(MaxTest) {
			vec2f v(10.0f, -10.0f);
			vec2f v2(12.0f, -7.0f);
			Assert::IsTrue(Max(v, v2) == vec2f(12.0f, -7.0f));
		}

		TEST_METHOD(SmoothStepTest) {
			vec2f edge0(2.0f, 2.0f);
			vec2f edge1(5.0f, 5.0f);
			vec2f t(3.0f, 3.0f);
			vec2f result = SmoothStep(edge0, edge1, t);
			Assert::IsTrue(vec2f::One*(7.0f/27.0f) == result);
		}
	};
}