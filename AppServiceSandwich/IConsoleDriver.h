#pragma once
#include <string>

class IConsoleDriver
{
public:
	virtual ~IConsoleDriver()
	{
	}

	virtual void WriteOutput(const std::string& buffer) = 0;
	virtual void WriteError(const std::string& buffer) = 0;
};