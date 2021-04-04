#pragma once
#include <string>

namespace ProcessCommand
{
	struct ExecutionResult
	{
		int exitCode;
		std::string output;
	};

	ExecutionResult Execute(const std::string& commandLine);
}