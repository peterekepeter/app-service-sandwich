#include "../AppServiceSandwichSrc/TestFramework.h"

begin_tests(AssertionsTest)
	
	test(assert_true_passes)
	{
		assert(true);
	}

	test(ensure_true_passes)
	{
		ensure(true);
	}

	test(expect_true_passes)
	{
		expect(true);
	}

	test(assert_false_throws_std_exception)
	{
		assert_throws(assert(false), std::exception);
	}

	test(assert_false_actally_throws_std_logic_error)
	{
		assert_throws(assert(false), std::logic_error);
	}

	test(ensure_false_throws)
	{
		assert_throws(ensure(false));
	}

	test(expect_false_throwsr)
	{
		assert_throws(expect(false));
	}

	test(single_arg_assert_contains_expression)
	{
		try {
			assert(3 == 4);
		}
		catch (std::logic_error error) {
			std::string message = error.what();
			assert(message.find("3 == 4") != std::string::npos)
		}
	}

	test(two_arg_assert_contains_custom_message)
	{
		try {
			assert(3 == 4, "inequality");
		}
		catch (std::logic_error error) {
			std::string message = error.what();
			assert(message.find("inequality") != std::string::npos)
		}
	}

end_tests(AssertionsTest)