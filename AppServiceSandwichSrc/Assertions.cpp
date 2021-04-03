#include "Assertions.h"

#include <sstream>

namespace assertions { 
namespace __details { 
namespace messages {

static std::string format_common(const char* pre, const char* message, const char* file, const int line)
{
	std::stringstream ss;
	ss << pre << message << "\n  at " << file << "(" << line << ")";
	return ss.str();
}

std::string format_assert(const char* message, const char* file, const int line)
{
	return format_common("Assertion: ", message, file, line);
}

std::string format_expect(const char* message, const char* file, const int line)
{
	return format_common("Expectation not met: ", message, file, line);
}

std::string format_ensure(const char* message, const char* file, const int line)
{
	return format_common("Failed to ensure that the ", message, file, line);
}

}}}

