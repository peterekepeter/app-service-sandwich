
#pragma once
#include "Console.hpp"

class Win32MessageBoxConsoleDriver : public IConsoleDriver
{
	bool showInfo = true;
	bool showError = true;
public:
	Win32MessageBoxConsoleDriver();
	~Win32MessageBoxConsoleDriver();
	bool GetShowInfo() const;
	void SetShowInfo(bool show_info);
	bool GetShowError() const;
	void SetShowError(bool show_error);
	void WriteOutput(const std::string& buffer) override;
	void WriteError(const std::string& buffer) override;
};
