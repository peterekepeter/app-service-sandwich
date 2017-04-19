#include <thread>
#include "DirectoryChangeService.hpp"

void DirectoryChangeService::_init(const char* directoryPath)
{
	this->signalWantToFinish = false;
	this->workerThread = std::thread([this]{
		while (!signalWantToFinish)
		{
			auto info = this->directoryChangeReader.Read();
			if (info.action != ChangeInformation::Action::None)
			{
				this->queueMutex.lock();
				queue.push(info);
				this->queueMutex.unlock();
			}
		}
	});
}

DirectoryChangeService::DirectoryChangeService(const std::string& directoryPath) : directoryChangeReader(directoryPath)
{
	_init(directoryPath.c_str());
}

DirectoryChangeService::DirectoryChangeService(const char* directoryPath) : directoryChangeReader(directoryPath)
{
	_init(directoryPath);
}

DirectoryChangeService::~DirectoryChangeService()
{
	this->signalWantToFinish = true;
	this->directoryChangeReader.CancelPendingRead();
	this->workerThread.join();
}

bool DirectoryChangeService::HasChange()
{
	this->queueMutex.lock();
	auto hasChange = queue.size() > 0;
	this->queueMutex.unlock();
	return hasChange;
}

ChangeInformation DirectoryChangeService::ReadChange()
{
	this->queueMutex.lock();
	auto data = queue.front();
	queue.pop();
	this->queueMutex.unlock();
	return data;
}

