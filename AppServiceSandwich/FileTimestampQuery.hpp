#pragma once
#include <string>

// Get the timestamp of a file, optinally wait until file is ready for reading
class FileTimestampQuery
{
private:
	void _init(const char* filePath, bool waitUntilReadyToRead, long long timeout = 0);

public:
	// true if the file exists
	bool fileExists;
	// the time when the file was created
	long long created;
	// the time when the file was last accessed
	long long accessed;
	// the time when the file was last written
	long long written;

	// how much time was spent on waiting (milliseconds)
	long long queryTime;
	// how many times the query was retried (file shared read violation)
	int retries;

	FileTimestampQuery(const std::string& filePath, bool waitUntilReadyToRead = true, long long timeout = 0);
	FileTimestampQuery(const char* filePath, bool waitUntilReadyToRead = true, long long timeout = 0);
};
