#pragma once
#include "ResourcePath.hpp"

class ITimestampingService
{
public:
	// return the timestamp of a file, if returned value <= 0 then there is no information avaliable
	virtual long long GetFileTimestamp(const ResourcePath& filePath) = 0;
	virtual ~ITimestampingService()	{	}
};
