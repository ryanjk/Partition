#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\Memory.h>
#include <Utilities\UtilityTypes.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using pn::linear_allocator;
using pn::frame_string;

namespace MemoryUnitTest {
TEST_CLASS(BasicFrameStringTest) {
public:

	TEST_METHOD(ConstructorTest) {
		linear_allocator a(5);
		frame_string::SetFrameAllocator(&a);

		frame_string s;
		Assert::IsTrue(s.data == nullptr);
		Assert::IsTrue(s.length == 0);
		Assert::IsTrue(a.HasFree());
		Assert::IsTrue(a.HasFree(5));

		frame_string s2("Hello");
		Assert::IsTrue(s2.data != nullptr);
		Assert::IsTrue(s2.length == 5);
		Assert::IsFalse(a.HasFree());
		Assert::AreEqual('H', s2[0]);
		Assert::AreEqual('e', s2[1]);
		Assert::AreEqual('l', s2[2]);
		Assert::AreEqual('l', s2[3]);
		Assert::AreEqual('o', s2[4]);

		frame_string s3(std::move(s2));
		Assert::IsTrue(s2.data == nullptr);
		Assert::IsTrue(s2.length == 0);

		Assert::IsTrue(s3.data != nullptr);
		Assert::IsTrue(s3.length == 5);
		Assert::IsFalse(a.HasFree());
		Assert::AreEqual('H', s3[0]);
		Assert::AreEqual('e', s3[1]);
		Assert::AreEqual('l', s3[2]);
		Assert::AreEqual('l', s3[3]);
		Assert::AreEqual('o', s3[4]);

		
	}



};
}