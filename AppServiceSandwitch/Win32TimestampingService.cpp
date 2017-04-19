#include "Win32TimestampingService.hpp"
#include "FileTimestampQuery.hpp"

long long Win32TimestampingService::GetFileTimestamp(const ResourcePath& filePath)
{
	FileTimestampQuery file_timestamp_query(filePath.ToCharPtr(), true, 0);
	if (file_timestamp_query.fileExists)
	{
		return file_timestamp_query.written;
	}
	else
	{
		return -1;
	}
}
