#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>
#include <unordered_map>
#include "../AppServiceSandwichSrc/ResourcePath.hpp"

namespace Test
{
	TEST_CLASS(ResourcePathTest)
	{
	public:

		TEST_METHOD(WindowsDriveLetterPath)
		{
			ResourcePath path("c:/program files/");
			auto string = path.ToString();
			Assert::IsTrue(string[0]=='c');
			Assert::IsTrue(string[1] == ':');
			ResourcePath relative("../license.txt"), combined, expected("c:/license.txt");
			combined = path + relative;
			Assert::IsTrue(combined == expected);
		}

		TEST_METHOD(IsFilePropertyTest)
		{
			ResourcePath p1("test.txt"), p2("./");
			Assert::IsTrue(p1.IsFilePath());
			Assert::IsFalse(p2.IsFilePath());
		}

		TEST_METHOD(IsDirectoryPropertyTest)
		{
			ResourcePath p1("scripts/"), p2("scripts/index.js");
			Assert::IsTrue(p1.IsDirectoryPath());
			Assert::IsFalse(p2.IsDirectoryPath());
		}

		TEST_METHOD(CaseInsensitiveTest)
		{
			ResourcePath p1("readme.TXT"), p2("readme.txt");
			Assert::IsTrue(p1 == p2);
		}

		TEST_METHOD(ConcatException) 
		{
			ResourcePath p1("readme.txt"), p2("./views/index.html");
			Assert::ExpectException<ResourcePath::Exception>([p1,p2] {
				auto p3 = p1 + p2;
				Assert::IsTrue(false); //unreachable
			});
		}

		TEST_METHOD(ConcatSimple)
		{
			ResourcePath p1("resources/textures/"), p2("wood.png");
			ResourcePath p3("resources/textures/wood.png");
			ResourcePath p4 = p1 + p2;
			Assert::IsTrue(p4 == p3);
		}

		TEST_METHOD(ConcatGoingUp)
		{
			ResourcePath p1("../../lib/code.h");
			ResourcePath p2("/project/src/logic/logger/");
			ResourcePath p3("/project/src/lib/code.h");
			auto p4 = p2 + p1;
			Assert::IsTrue(p4 == p3);
		}

		TEST_METHOD(Normalization)
		{
			ResourcePath p1(".\\a\\b\\..\\c\\..\\..\\readme.txt");
			ResourcePath p2("./q/../readme.txt");
			Assert::IsTrue(p1 == p2);
		}

		TEST_METHOD(OrderedMap)
		{
			std::map<ResourcePath, int> map;
			map[ResourcePath("a/a.txt")] = 1;
			map[ResourcePath("a/b.txt")] = 2;
			Assert::AreEqual(map[ResourcePath("a/a.txt")], 1);
			Assert::AreEqual(map[ResourcePath("a/b.txt")], 2);
		}

		TEST_METHOD(UnorderedMap)
		{
			std::unordered_map<ResourcePath, int, ResourcePath::Hasher> map;
			map[ResourcePath("a/a.txt")] = 1;
			map[ResourcePath("a/b.txt")] = 2;
			Assert::AreEqual(map[ResourcePath("a/a.txt")], 1);
			Assert::AreEqual(map[ResourcePath("a/b.txt")], 2);
		}


	};
}