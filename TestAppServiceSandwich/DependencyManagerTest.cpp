#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>
#include <unordered_map>
#include "../AppServiceSandwich/DependencyManager.hpp"

namespace Test
{
	TEST_CLASS(DependencyManagerTest)
	{
	public:
		TEST_METHOD(BasicDependency)
		{
			DependencyManager manager;
			int number = 42;
			manager.For<int>().UseInstance(&number);
			auto x = manager.GetInstance<int>();
			Assert::IsTrue(*x == 42);
		}

		class IClassWithDefaultConstructor
		{
		public:
			virtual ~IClassWithDefaultConstructor()
			{
			}

			virtual const std::string& GetName() = 0;
		};

		class ClassWithDefaultConstructor : public IClassWithDefaultConstructor
		{
			std::string name;
		public:
			virtual ~ClassWithDefaultConstructor()
			{
			}

			const std::string& GetName() override
			{
				return name;
			}

			ClassWithDefaultConstructor()
			{
				name = "test";
			}
		};

		TEST_METHOD(DefaultConstructor)
		{
			DependencyManager manager;
			manager.For<ClassWithDefaultConstructor>().UseDefaultConstructor();
			auto x = manager.GetInstance<ClassWithDefaultConstructor>();
			Assert::IsTrue(x->GetName() == "test");
		}

		TEST_METHOD(AbstractDependencyConreteType)
		{
			DependencyManager manager;
			manager.For<ClassWithDefaultConstructor>().UseDefaultConstructor();
			manager.For<IClassWithDefaultConstructor>().UseType<ClassWithDefaultConstructor>();
			auto x = manager.GetInstance<IClassWithDefaultConstructor>();
			Assert::IsTrue(x->GetName() == "test");
		}

		TEST_METHOD(UserDefinedFactory)
		{
			DependencyManager manager;
			int number = 42;
			manager.For<int>().UseInstance(&number);
			manager.For<ClassWithConstructor>().UseConstructor<int>();
			auto x = manager.GetInstance<ClassWithConstructor>();
			Assert::IsTrue(x->x == 42);
		}

		class ClassWithConstructor
		{
		public:
			int x;

			explicit ClassWithConstructor(int* x)
				: x(*x)
			{
			}
		};
	};
}
