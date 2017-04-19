#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include "DirectoryChangeReader.hpp"

// a high level service built on directory change reader
// encapsulates a reader thread
class DirectoryChangeService
{
	DirectoryChangeReader directoryChangeReader;
	void _init(const char* directoryPath);
	std::queue<ChangeInformation> queue;
	std::mutex queueMutex;
	std::thread workerThread;
	bool signalWantToFinish;
public:
	DirectoryChangeService(const std::string& directoryPath);
	DirectoryChangeService(const char* directoryPath);
	~DirectoryChangeService();

	// see if there are any changes queued
	bool HasChange();

	// read the changes
	ChangeInformation ReadChange();

};
