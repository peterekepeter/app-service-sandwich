#include "Win32DefaultConsoleDriver.hpp"
#include <Windows.h>

Win32DefaultConsoleDriver::Win32DefaultConsoleDriver()
{
	AllocConsole();
}

Win32DefaultConsoleDriver::~Win32DefaultConsoleDriver()
{
	FreeConsole();
}

void Win32DefaultConsoleDriver::SetTitle(const std::string& buffer)
{
	SetConsoleTitleA(buffer.c_str());
}

void Win32DefaultConsoleDriver::WriteOutput(const std::string& buffer)
{
	if (buffer.size() == 0) {
		return;
	}
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written = 0;
	SetConsoleTextAttribute(handle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	WriteConsoleA(handle, buffer.c_str(), buffer.size(), &written, nullptr);
}

void Win32DefaultConsoleDriver::WriteError(const std::string& buffer)
{
	if (buffer.size() == 0) {
		return;
	}
	auto handle = GetStdHandle(STD_ERROR_HANDLE);
	DWORD written = 0;
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
	WriteConsoleA(handle, buffer.c_str(), buffer.size(), &written, nullptr);
}
