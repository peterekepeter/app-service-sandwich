
#pragma once
#include "Console.hpp"

class Win32DefaultConsoleDriver : public IConsoleDriver
{
public:
	Win32DefaultConsoleDriver();
	~Win32DefaultConsoleDriver();
	void SetTitle(const std::string& buffer);
	void WriteOutput(const std::string& buffer) override;
	void WriteError(const std::string& buffer) override;
};
