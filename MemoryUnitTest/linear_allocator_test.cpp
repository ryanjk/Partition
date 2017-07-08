#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Memory.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using pn::linear_allocator;

namespace MemoryUnitTest {
TEST_CLASS(LinearAllocatorTest) {
public:
	struct blob {
		int i; int j; int k; double d; float f;
		blob() {}
		blob(int i, int j, int k, double d, float f) : i(i), j(j), k(k), d(d), f(f) {

		}
	};

	TEST_METHOD(ConstructorTest) {
		linear_allocator<1024> a;
		Assert::IsTrue(a.HasFree(1024));
		Assert::IsTrue(a.HasFree());
	}

	TEST_METHOD(CreateAndDestroyTest) {
		linear_allocator<16> a;

		auto* b = a.Allocate(4);
		Assert::IsFalse(a.HasFree(16));
		Assert::IsTrue(a.HasFree(12));

		int* i = a.Create<int>(5);
		Assert::AreEqual(5, *i);
		Assert::IsTrue(a.HasFree(8));

		auto* c = a.Allocate(8);
		Assert::IsFalse(a.HasFree());

		a.Release();
		Assert::IsTrue(a.HasFree());
		Assert::IsTrue(a.HasFree(16));
	}



};
}