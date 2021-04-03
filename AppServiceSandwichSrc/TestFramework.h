#pragma once
#include "Assertions.h"
#include "CppUnitTest.h"

#define begin_tests(CLASS) namespace Test { TEST_CLASS(CLASS){ public: void __nope(){ try {}
#define test(TNAME) catch(std::exception error){ Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(error.what()); throw error; }} TEST_METHOD(TNAME) { try
#define end_tests(CLASS) catch(std::exception error){ Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(error.what()); throw error; }}};}

#define __TEST_FRAMEWORK_ASSERT_THROWS_2(EXPRESSION, EXCEPTION_TYPE) {bool exception_was_thrown=false;try{EXPRESSION;}catch(EXCEPTION_TYPE error){exception_was_thrown=true;};ensure(exception_was_thrown);}
#define __TEST_FRAMEWORK_ASSERT_THROWS_1(EXPRESSION) __TEST_FRAMEWORK_ASSERT_THROWS_2(EXPRESSION, std::exception)

#define assert_throws(...) __MACRO_VA_EXPAND( __MACRO_3RD_ARG(__VA_ARGS__, __TEST_FRAMEWORK_ASSERT_THROWS_2, __TEST_FRAMEWORK_ASSERT_THROWS_1)(__VA_ARGS__))