#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Memory.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using pn::pool_allocator;

namespace MemoryUnitTest
{		
	TEST_CLASS(PoolAllocatorTest)
	{
	public:
		struct blob {
			int i; int j; int k; double d; float f;
			blob() {}
			blob(int i, int j, int k, double d, float f) : i(i), j(j), k(k), d(d), f(f) {
				
			}
		};
		
		TEST_METHOD(ConstructorTest) {
			{
				pool_allocator<char, 5> a;
				Assert::IsTrue(a.HasFree());
			}

			{
				pool_allocator<int, 50> a;
				Assert::IsTrue(a.HasFree());
			}

			{
				pool_allocator<blob, 500> a;
				Assert::IsTrue(a.HasFree());
			}
		}

		TEST_METHOD(CreateAndDestroyTest) {
			pool_allocator<blob, 2> a;

			blob* b = a.Create();
			Assert::IsTrue(b != nullptr);
			Assert::IsTrue(a.HasFree());

			blob* c = a.Create(1, 2, 3, 10.4, 15.4f);
			Assert::IsTrue(c != nullptr);
			Assert::IsFalse(a.HasFree());
			Assert::AreEqual(1, c->i);
			Assert::AreEqual(2, c->j);
			Assert::AreEqual(3, c->k);
			Assert::AreEqual(10.4, c->d);
			Assert::AreEqual(15.4f, c->f);

			blob* d = a.Create();
			Assert::IsTrue(d == nullptr);
			Assert::IsFalse(a.HasFree());

			a.Release(c);
			Assert::IsTrue(a.HasFree());
		}



	};
}