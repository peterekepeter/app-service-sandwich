#pragma once
#include <array>
#include "ChangeInformation.hpp"

// low level class that can 'read' the changes that happen in a directory tree
class DirectoryChangeReader
{
public:
	// blocks the caller thread until a change is avaliable, and returns information about that change
	ChangeInformation Read();

	// unlbocks the caller stuck at reading, read will return an action None, used for sync
	void CancelPendingRead();

	DirectoryChangeReader(const char* directoryPath);

	DirectoryChangeReader(const std::string& directoryPath);

	~DirectoryChangeReader();

private:
	void* overlappedEvent;
	void* cancelEvent;
	void* directoryHandle;
	std::array<unsigned char, 65536> buffer;
	unsigned long bufferBytesUsed;
	unsigned long entryOffset;

	void _init(const char* directoryPath);

};
