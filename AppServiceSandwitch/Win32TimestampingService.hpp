#pragma once
#include "ITimestampingService.hpp"

class Win32TimestampingService :public ITimestampingService
{
public:
	long long GetFileTimestamp(const ResourcePath& filePath) override;
	~Win32TimestampingService() {};
};
