#include "CppUnitTest.h"
#include "AutoBuild.hpp"
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace Test
{
	TEST_CLASS(AutoBuildTest)
	{
	public:
		TEST_METHOD(is_default_constructible)
		{
			AutoBuild atb;
		}

		TEST_METHOD(can_define_tool_and_step)
		{
			AutoBuild atb;
			atb.tool("awesome", [](const char* in, const char* out) { return AutoBuild::RESULT_SUCCESS; });
			atb.step("awesome", "a.md", "a.html");
		}
		
		TEST_METHOD(tool_gets_called_automatically)
		{
			AutoBuild atb;
			int count = 0;
			atb.tool("awesome", [&count](const char* in, const char* out) { count++; return AutoBuild::RESULT_SUCCESS; });
			atb.step("awesome", "a.md", "a.html");
			atb.wait_idle();
			Assert::AreEqual(1, count, L"step should be called automatically once");
		}

		TEST_METHOD(all_independent_steps_execute)
		{
			AutoBuild atb;
			std::vector<std::string> order;
			atb.tool("copy", [&](const char* in, const char* out) { order.push_back(in); return AutoBuild::RESULT_SUCCESS; });
			atb.step("copy", "a", "a.out");
			atb.step("copy", "b", "b.out");
			atb.step("copy", "c", "c.out");
			atb.wait_idle();
			Assert::AreEqual(3, (int)order.size());
			Assert::AreEqual("a", order[0].c_str(), "'a' should be processed first");
			Assert::AreEqual("b", order[1].c_str());
			Assert::AreEqual("c", order[2].c_str());
		}

		TEST_METHOD(execution_order_based_on_dependency)
		{
			AutoBuild atb;
			std::vector<std::string> order;
			atb.tool("copy", [&](const char* in, const char* out) { order.push_back(in); return AutoBuild::RESULT_SUCCESS; });
			atb.step("copy", "c", "d");
			atb.step("copy", "b", "c");
			atb.step("copy", "a", "b");
			atb.wait_idle();
			Assert::AreEqual(3, (int)order.size());
			Assert::AreEqual("a", order[0].c_str(), L"'a' should be processed first");
			Assert::AreEqual("b", order[1].c_str());
			Assert::AreEqual("c", order[2].c_str());
		}

		TEST_METHOD(dependencies_not_executed_of_failed_steps)
		{
			AutoBuild atb;
			std::vector<std::string> order;
			atb.tool("copy", [&](const char* in, const char* out) { order.push_back(in); return AutoBuild::RESULT_FAILURE; });
			atb.step("copy", "a", "b");
			atb.step("copy", "b", "c");
			atb.wait_idle();
			Assert::AreEqual(1, (int)order.size());
			Assert::AreEqual("a", order[0].c_str(), L"'a' should be processed first");
		}

		TEST_METHOD(is_idle_updates_correctly)
		{
			AutoBuild atb;
			Assert::AreEqual(true, atb.m_is_idle, L"initially it's empty so its idle");
			atb.tool("copy", [&](const char* in, const char* out) { return AutoBuild::RESULT_FAILURE; });
			atb.step("copy", "a", "b");
			atb.step("copy", "b", "c");
			Assert::AreEqual(false, atb.m_is_idle, L"steps didn't run yet, so it shouln't be idle");
			atb.wait_idle();
			Assert::AreEqual(true, atb.m_is_idle, L"should always be idle after wait_idle");
		}

		TEST_METHOD(m_is_success)
		{
			AutoBuild atb;
			Assert::AreEqual(true, atb.m_is_success, L"you can do no wrong at this point");
			atb.tool("copy", [&](const char* in, const char* out) { return AutoBuild::RESULT_FAILURE; });
			atb.step("copy", "a", "b");
			atb.step("copy", "b", "c");
			atb.wait_idle();
			Assert::AreEqual(false, atb.m_is_success, L"build steps contain error");
			atb.tool("copy", [&](const char* in, const char* out) { return AutoBuild::RESULT_SUCCESS; });
			atb.wait_idle();
			Assert::AreEqual(true, atb.m_is_success, L"should be successful now");
		}

		TEST_METHOD(stats_are_updated)
		{
			AutoBuild atb;
			Assert::AreEqual(true, atb.m_is_idle, L"initially it's empty so its idle");
			atb.tool("copy", [&](const char* in, const char* out) { return AutoBuild::RESULT_FAILURE; });
			atb.step("copy", "a", "b");
			atb.step("copy", "b", "c");
			atb.wait_idle();
			Assert::AreEqual(1, atb.m_completed_step_count, L"Only 1 step is run, since if fails rest does not run");
			Assert::AreEqual(2, atb.m_total_step_count, L"Should be exactly 2 steps");
			Assert::AreEqual(1, atb.m_failed_step_count, L"First step should fail.");
			atb.tool("copy", [&](const char* in, const char* out) { return AutoBuild::RESULT_SUCCESS; });
			atb.wait_idle();
			Assert::AreEqual(2, atb.m_completed_step_count, L"Both steps should complete now");
			Assert::AreEqual(2, atb.m_total_step_count, L"Should be exactly 2 steps");
			Assert::AreEqual(0, atb.m_failed_step_count, L"No step should fail");
		}



	};
}