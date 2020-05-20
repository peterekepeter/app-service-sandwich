#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>
#include <unordered_map>
#include "DependencyManager.hpp"

namespace Test
{
	TEST_CLASS(DependencyManagerAutoFactoryTest)
	{
	public:
		class TypeWithDefaultConstructor
		{
		public:
			int x = 42;
		};

		TEST_METHOD(AutoCreateInstancesWithDefaultConstructor)
		{
			DependencyManager di;
			auto instance = di.GetInstance<TypeWithDefaultConstructor>();
			Assert::AreEqual(instance->x, 42);
		}

		class TypeWithDependencyConstructor
		{
		public:
			int x;

			TypeWithDependencyConstructor(DependencyManager* di) {
				auto instance = di->GetInstance< TypeWithDefaultConstructor>();
				x = instance->x * 2;
			}
		};

		TEST_METHOD(AutoCreateInstanceWithDependencyConstructor)
		{
			DependencyManager di;
			auto instance = di.GetInstance<TypeWithDependencyConstructor>();
			Assert::AreEqual(instance->x, 84);
		}
	};
}