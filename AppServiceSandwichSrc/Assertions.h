#pragma once
#include <string>
#include "MacroHelpers.h"

#define __ASSERTIONS_STRINGIZE_DETAIL(X) #X
#define __ASSERTIONS_STRINGIZE(X) __ASSERTIONS_STRINGIZE_DETAIL(X)

#define __ASSERTIONS_THROW_IF(C, M) {if(!(C)) throw std::logic_error(M); }

// assuming string pooling is enabled each assert/expect/ensure should only add 1 string per file
namespace assertions { namespace __details { namespace messages {
	std::string format_assert(const char* message, const char* file, int line);
	std::string format_expect(const char* message, const char* file, int line);
	std::string format_ensure(const char* message, const char* file, int line);
}}}

#define __ASSERTIONS_MESSAGE(F, M) (assertions::__details::messages::F(M, __FILE__, __LINE__))

#define __ASSERTIONS_ASSERT_2(X,M) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_assert, M))
#define __ASSERTIONS_ASSERT_1(X) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_assert, __ASSERTIONS_STRINGIZE(X)))

#define __ASSERTIONS_EXPECT_2(X,M) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_expect, M))
#define __ASSERTIONS_EXPECT_1(X) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_expect, __ASSERTIONS_STRINGIZE(X)))

#define __ASSERTIONS_ENSURE_2(X,M) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_ensure, M))
#define __ASSERTIONS_ENSURE_1(X) __ASSERTIONS_THROW_IF(X, __ASSERTIONS_MESSAGE(format_ensure, __ASSERTIONS_STRINGIZE(X)))

#define assert(...) __MACRO_VA_EXPAND( __MACRO_3RD_ARG(__VA_ARGS__, __ASSERTIONS_ASSERT_2, __ASSERTIONS_ASSERT_1)(__VA_ARGS__) )
#define expect(...) __MACRO_VA_EXPAND( __MACRO_3RD_ARG(__VA_ARGS__, __ASSERTIONS_EXPECT_2, __ASSERTIONS_EXPECT_1)(__VA_ARGS__) )
#define ensure(...) __MACRO_VA_EXPAND( __MACRO_3RD_ARG(__VA_ARGS__, __ASSERTIONS_ENSURE_2, __ASSERTIONS_ENSURE_1)(__VA_ARGS__) )