#include <Windows.h>
#include <stdexcept>
#include <codecvt>
#include "DirectoryChangeReader.hpp"

//setup converter
using convert_type = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_type, wchar_t> converter;

void DirectoryChangeReader::_init(const char* directoryPath)
{
	overlappedEvent = CreateEvent(nullptr, true, false, nullptr);
	cancelEvent = CreateEvent(nullptr, true, false, nullptr);

	bufferBytesUsed = 0;
	entryOffset = 0;
	directoryHandle = CreateFileA(
		directoryPath,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr
	);
	if (directoryHandle == INVALID_HANDLE_VALUE)
	{
		auto error = GetLastError();
		char message[4096];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, message, 4096, nullptr);
		throw std::invalid_argument(std::string("Cannot open directory ")+ directoryPath + " " + message);
	}
}

DirectoryChangeReader::DirectoryChangeReader(const char* directoryPath)
{
	_init(directoryPath);
}

DirectoryChangeReader::DirectoryChangeReader(const std::string& directoryPath)
{
	_init(directoryPath.c_str());
}

DirectoryChangeReader::~DirectoryChangeReader()
{
	CloseHandle(overlappedEvent);
	CloseHandle(cancelEvent);
	CloseHandle(directoryHandle);
}

ChangeInformation DirectoryChangeReader::Read()
{
	if (entryOffset >= bufferBytesUsed)
	{
		DWORD transfered;
		OVERLAPPED overlapped;
		ZeroMemory(&overlapped, sizeof(OVERLAPPED));
		
		ResetEvent(overlappedEvent);
		ResetEvent(cancelEvent);
		overlapped.hEvent = overlappedEvent;

		auto success = ReadDirectoryChangesW(
			directoryHandle,
			buffer.data(),
			buffer.size(),
			true,
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
			&bufferBytesUsed,
			&overlapped,
			nullptr
		);

		if (!success)
		{
			auto error = GetLastError();
			char message[4096];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, message, 4096, nullptr);
			throw std::runtime_error(std::string("ReadDirectoryChangesW failed, ") + message);
		}
		
		HANDLE handles[2] = { overlappedEvent, cancelEvent };

		auto result = WaitForMultipleObjects(2, handles, false, INFINITE);
		if (result == WAIT_OBJECT_0)
		{
			// get read results and continue parsing
			result = GetOverlappedResult(directoryHandle, &overlapped, &transfered, FALSE);
			if (result == FALSE)
			{
				// fail
				auto error = GetLastError();
				char message[4096];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, message, 4096, nullptr);
				throw std::runtime_error(std::string("GetOverlappedResult failed, ") + message);
			}
			// continue to parsing
			bufferBytesUsed = transfered;
			entryOffset = 0;
		} 
		else if (result == WAIT_OBJECT_0 + 1)
		{
			// user cancel
			result = CancelIoEx(directoryHandle, &overlapped);
			if (result == TRUE || GetLastError() != ERROR_NOT_FOUND)
			{
				// wait for system to terminate IO
				result = GetOverlappedResult(directoryHandle, &overlapped, &transfered, TRUE);
				if (result == FALSE)
				{
					// fail
					auto error = GetLastError();
					if (error != 995) // 995 means that error because application exit, which is our general case here, so don't throw in that case
					{
						char message[4096];
						FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, message, 4096, nullptr);
						throw std::runtime_error(std::string("GetOverlappedResult failed, ") + message);
					}
				}
			}
			return ChangeInformation(ChangeInformation::Action::None, "");
		} 
		else
		{
			throw std::runtime_error("DirectoryChangeReader::Read failed, unhandled application state");
		}

	}
	auto info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data() + entryOffset);
	if (info->NextEntryOffset == 0)
	{
		entryOffset = bufferBytesUsed;
	}
	else
	{
		entryOffset += info->NextEntryOffset;
	}
	std::wstring wideFilename(info->FileName, info->FileNameLength >> 1);
	auto filename = converter.to_bytes(wideFilename);
	auto action = ChangeInformation::Action(info->Action);
	return ChangeInformation(std::move(action), std::move(filename));
}

void DirectoryChangeReader::CancelPendingRead()
{
	// signal a thread blocked in by reading
	SetEvent(cancelEvent);
}
