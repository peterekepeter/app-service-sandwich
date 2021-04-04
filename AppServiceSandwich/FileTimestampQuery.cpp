#include <Windows.h>
#include <stdexcept>
#include "FileTimestampQuery.hpp"

static long long FiletimeToLongLong(const FILETIME& filetime)
{
	ULARGE_INTEGER large;
	large.HighPart = filetime.dwHighDateTime;
	large.LowPart = filetime.dwLowDateTime;
	return large.QuadPart;
}

void FileTimestampQuery::_init(const char* filePath, bool waitUntilReadyToRead, long long timeout)
{
	retries = 0;
	queryTime = 0;
	DWORD timeToWait = 2;
	while (true)
	{
		auto fileHandle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr,
		                              OPEN_EXISTING, 0, nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			if (waitUntilReadyToRead && GetLastError() == ERROR_SHARING_VIOLATION)
			{
				Sleep(timeToWait);
				queryTime += timeToWait;
				if (timeout == 0 || queryTime < timeout)
				{
					timeToWait *= 2;
					retries++;
					continue;
				}
			}
			auto error = GetLastError();
			if (error == ERROR_FILE_NOT_FOUND)
			{
				fileExists = false;
				created = 0;
				accessed = 0;
				written = 0;
			}
			else
			{
				char message[4096];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, LANG_SYSTEM_DEFAULT, message, 4096, nullptr);
				throw std::runtime_error(std::string("CreateFileA failed, ") + message);
			}
		}
		else
		{
			FILETIME c, a, w;
			GetFileTime(fileHandle, &c, &a, &w);
			CloseHandle(fileHandle);
			fileExists = true;
			created = FiletimeToLongLong(c);
			accessed = FiletimeToLongLong(a);
			written = FiletimeToLongLong(w);
			break;
		}
	}
}

FileTimestampQuery::FileTimestampQuery(const std::string& filePath, bool waitUntilReadyToRead, long long timeout)
{
	_init(filePath.c_str(), waitUntilReadyToRead, timeout);
}

FileTimestampQuery::FileTimestampQuery(const char* filePath, bool waitUntilReadyToRead, long long timeout)
{
	_init(filePath, waitUntilReadyToRead, timeout);
}
