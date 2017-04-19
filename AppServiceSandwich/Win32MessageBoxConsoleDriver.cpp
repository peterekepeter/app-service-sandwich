#include "Win32MessageBoxConsoleDriver.hpp"
#include "Windows.h"
Win32MessageBoxConsoleDriver::Win32MessageBoxConsoleDriver()
{
}

Win32MessageBoxConsoleDriver::~Win32MessageBoxConsoleDriver()
{
}

bool Win32MessageBoxConsoleDriver::GetShowInfo() const
{
	return showInfo;
}

void Win32MessageBoxConsoleDriver::SetShowInfo(bool show_info)
{
	showInfo = show_info;
}

bool Win32MessageBoxConsoleDriver::GetShowError() const
{
	return showError;
}

void Win32MessageBoxConsoleDriver::SetShowError(bool show_error)
{
	showError = show_error;
}

void Win32MessageBoxConsoleDriver::WriteOutput(const std::string& buffer)
{
	if (showInfo && buffer.length() > 0)
	{
		MessageBoxA(nullptr, buffer.c_str(), "Information", MB_ICONINFORMATION | MB_OK);
	}
}

void Win32MessageBoxConsoleDriver::WriteError(const std::string& buffer)
{
	if (showError && buffer.length() > 0)
	{
		MessageBoxA(nullptr, buffer.c_str(), "Error", MB_ICONERROR | MB_OK);
	}
}
