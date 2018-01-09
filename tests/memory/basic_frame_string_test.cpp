#include "stdafx.h"
#include "CppUnitTest.h"

#include <Utilities\UtilityTypes.h>
#include <Utilities\frame_string.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using pn::linear_allocator;
using pn::frame_string;

namespace MemoryUnitTest {
TEST_CLASS(BasicFrameStringTest) {
public:

	TEST_METHOD(ConstructorTest) {
		linear_allocator a(5);
		frame_string::SetFrameAllocator(&a);

		{
			// default constructor
			frame_string s;
			Assert::IsTrue(s.data == nullptr);
			Assert::IsTrue(s.length == 0);
			Assert::IsTrue(a.HasFree());
			Assert::IsTrue(a.HasFree(5));

			// const char* constructor
			frame_string s2("Hello");
			Assert::IsTrue(s2.data != nullptr);
			Assert::IsTrue(s2.length == 5);
			Assert::IsFalse(a.HasFree());
			Assert::AreEqual('H', s2[0]);
			Assert::AreEqual('e', s2[1]);
			Assert::AreEqual('l', s2[2]);
			Assert::AreEqual('l', s2[3]);
			Assert::AreEqual('o', s2[4]);

			// move constructor
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

		a.Release();

		{
			frame_string s("ba");
			Assert::IsTrue(a.HasFree(3));

			// copy constructor
			frame_string s2(s);
			Assert::IsTrue(a.HasFree(1));
			Assert::IsTrue(s[0] == s2[0]);
			Assert::IsTrue(s[1] == s2[1]);
		}

		
	}

	TEST_METHOD(EqualityTest) {
		linear_allocator a(1024);
		frame_string::SetFrameAllocator(&a);

		frame_string s("this is a long string \n and it's a lot of words...");
		auto s2 = s;

		Assert::IsTrue(s == "this is a long string \n and it's a lot of words...");

		Assert::IsTrue(s == s2);
		Assert::IsFalse(s != s2);

		frame_string s3("this is a string.");
		Assert::IsFalse(s == s3);
		Assert::IsTrue(s != s3);

		std::string stds("this is a string.");
		Assert::IsTrue(stds == s3);
	}

	TEST_METHOD(EmptyStringTest) {
		linear_allocator a(1024);
		frame_string::SetFrameAllocator(&a);

		frame_string s;
		frame_string s2("");

		Assert::IsTrue(s == s2);
		Assert::IsTrue(s == "");
		Assert::IsTrue(0 == s.length);
		Assert::IsTrue(0 == s2.length);
	}

	TEST_METHOD(ConcatenationTest) {
		linear_allocator a(20);
		frame_string::SetFrameAllocator(&a);

		{
			frame_string s("thisa");
			frame_string s2("THISA");
			auto b = s + s2;

			Assert::IsTrue(b == "thisaTHISA");
			Assert::IsFalse(a.HasFree());
		}

		a.Release();

		{
			frame_string s("thisa");
			auto b = "THISA" + s;

			Assert::IsTrue(b == "THISAthisa");
			Assert::IsTrue(a.HasFree(5));
		}

		a.Release();

		{
			frame_string s("thisa");
			pn::string s2("THISA");
			auto b = s + s2;

			Assert::IsTrue(b == "thisaTHISA");
			Assert::IsTrue(a.HasFree(5));
		}

	}



};
}