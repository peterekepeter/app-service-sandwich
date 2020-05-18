#include "ProcessCommand.hpp"

#include <stdexcept>
#include <vector>
#include <Windows.h>

// good docs
// https://docs.microsoft.com/en-us/windows/win32/ProcThread/creating-a-child-process-with-redirected-input-and-output

#define ErrorExit(A) throw std::runtime_error(A);

struct IoHandles
{
	HANDLE child_stdin_read = NULL;
	HANDLE child_stdin_write = NULL;
	HANDLE child_stdout_read = NULL;
	HANDLE child_stdout_write = NULL;
};

static IoHandles CreateIoPipes();
void CheckedCloseHandle(HANDLE handle);
void WriteToPipe(const std::string data, HANDLE pipe);
void WriteToPipeAndClose(const std::string data, HANDLE pipe);
std::string ReadDataFromPipeAndClose(HANDLE pipe);
std::string ReadDataFromPipe(HANDLE pipe);

PROCESS_INFORMATION CreateChildProcess(
	const std::string& commandLine,
	IoHandles& ioPipes);

ProcessCommand::ExecutionResult ProcessCommand::Execute(
	const std::string& commandLine)
{
	auto ioPipes = CreateIoPipes();
	auto process = CreateChildProcess(commandLine, ioPipes);
	CheckedCloseHandle(ioPipes.child_stdout_write);
	CheckedCloseHandle(ioPipes.child_stdin_read);

	WriteToPipeAndClose("", ioPipes.child_stdin_write);
	auto result = ReadDataFromPipeAndClose(ioPipes.child_stdout_read);
	DWORD childExitCode = STILL_ACTIVE;
	GetExitCodeProcess(process.hProcess, &childExitCode);
	if (childExitCode == STILL_ACTIVE) {
		WaitForSingleObject(process.hProcess, INFINITE);
		GetExitCodeProcess(process.hProcess, &childExitCode);
	}
	CheckedCloseHandle(process.hProcess);
	CheckedCloseHandle(process.hThread);
	int code = childExitCode;
	return ExecutionResult{ code, result };
}

void WriteToPipeAndClose(const std::string data, HANDLE pipe)
{
	WriteToPipe(data, pipe);
	CheckedCloseHandle(pipe);
}

void WriteToPipe(const std::string data, HANDLE pipe)
{
	if (data.length() == 0) {
		return; // nothing to write
	}
	DWORD written = 0;
	if (!WriteFile(pipe, data.data(), data.length(), &written, NULL)) {
		throw std::runtime_error("Failed to write data to pipe.");
	}
	if (written != data.length()) {
		throw std::runtime_error("Did not write all data to pipe.");
	}
}

static IoHandles CreateIoPipes() {

	SECURITY_ATTRIBUTES saAttr;
	IoHandles result;

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;


	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&result.child_stdout_read, &result.child_stdout_write, &saAttr, 0))
		ErrorExit(TEXT("StdoutRd CreatePipe"));

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(result.child_stdout_read, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&result.child_stdin_read, &result.child_stdin_write, &saAttr, 0))
		ErrorExit(TEXT("Stdin CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(result.child_stdin_write, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdin SetHandleInformation"));

	return result;
}

PROCESS_INFORMATION CreateChildProcess(
	const std::string& commandLine,
	IoHandles& ioPipes)
{
	// Create child
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = ioPipes.child_stdout_write;
	siStartInfo.hStdOutput = ioPipes.child_stdout_write;
	siStartInfo.hStdInput = ioPipes.child_stdin_read;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// might be able to factor out the copy, 
	// but for now, leave it for safety
	std::vector<char> cmd;
	for (char c : commandLine) {
		cmd.push_back(c);
	}
	cmd.push_back(0);

	bSuccess = CreateProcess(NULL,
		cmd.data(),     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	 // If an error occurs, exit the application. 
	if (!bSuccess)
	{
		LPVOID lpMsgBuf;
		auto error =GetLastError();
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)& lpMsgBuf,
			0, NULL);
		ErrorExit(TEXT("CreateProcess"));
	}
	return piProcInfo;
}

void CheckedCloseHandle(HANDLE handle)
{
	if (!CloseHandle(handle)) {
		throw new std::runtime_error("Failed to close handle.");
	}
}

std::string ReadDataFromPipeAndClose(HANDLE pipe)
{
	auto data = ReadDataFromPipe(pipe);
	CheckedCloseHandle(pipe);
	return data;
}

std::string ReadDataFromPipe(HANDLE pipe)
{
	const size_t BUFSIZE = 4096;
	DWORD dwRead;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	std::string result;

	for (;;)
	{
		bSuccess = ReadFile(pipe, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		for (DWORD i = 0; i < dwRead; i++) {
			result.push_back(chBuf[i]);
		}
	}

	return result;
}