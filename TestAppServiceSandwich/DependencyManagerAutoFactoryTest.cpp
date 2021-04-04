#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>
#include <unordered_map>
#include "../AppServiceSandwich/DependencyManager.hpp"

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

		TEST_METHOD(auto_create_instances_with_default_constructor)
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
				auto instance = di->GetInstance<TypeWithDefaultConstructor>();
				x = instance->x * 2;
			}
		};

		TEST_METHOD(auto_create_instance_with_dependency_constructor)
		{
			DependencyManager di;
			auto instance = di.GetInstance<TypeWithDependencyConstructor>();
			Assert::AreEqual(instance->x, 84);
		}

		class TypeWithBothConstructors
		{
		public:
			const char* mode;

			TypeWithBothConstructors() {
				mode = "default constructor";
			}

			TypeWithBothConstructors(DependencyManager* di) {
				mode = "dependency constructor";
			}
		};

		TEST_METHOD(auto_create_instance_prefers_di_constructor)
		{
			DependencyManager di;
			auto instance = di.GetInstance<TypeWithBothConstructors>();
			Assert::AreEqual(instance->mode, "dependency constructor");
		}

	};
}