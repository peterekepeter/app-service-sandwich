#include "ResourceManager.hpp"
#include "ResourcePath.hpp"


ManagedResourceHolder::ManagedResourceHolder(): reference_count(0), basePtr(nullptr), derivedPtr(nullptr), timestamp(0)
{
	// don't try this at home, kids!
}

ManagedResourceHolder::ManagedResourceHolder(ManagedResource* resource, void* derived): reference_count(0),
                                                                                          basePtr(resource), derivedPtr(derived), timestamp(0)
{
}

ManagedResourceHolder::ManagedResourceHolder(ManagedResourceHolder&& other) noexcept: reference_count(other.reference_count),
                                                                                                       basePtr(other.basePtr), derivedPtr(other.derivedPtr), timestamp(other.timestamp)
{
	other.basePtr = nullptr;
	other.derivedPtr = nullptr;
}

ManagedResourceHolder& ManagedResourceHolder::operator=(ManagedResourceHolder&& other) noexcept
{
	if (this == &other)
		return *this;
	reference_count = other.reference_count;
	basePtr = other.basePtr;
	timestamp = other.timestamp;
	derivedPtr = other.derivedPtr;
	other.basePtr = nullptr;
	return *this;
}

ManagedResourceHolder::~ManagedResourceHolder()
{
	if (basePtr != nullptr)
	{
		delete basePtr;
		basePtr = nullptr;
		derivedPtr = nullptr;
	}
}

ResourceManager::ManagedResourceType::ManagedResourceType(): name(""), factory(nullptr)
{
}

ResourceManager::ManagedResourceType::ManagedResourceType(const std::string& name, ResourceFactory& factory): name(name), factory(&factory)
{
}

void ResourceManager::CallFactory(ResourceManager::ManagedResourceType& type, const ResourcePath& resource_path, ManagedResource*& resource)
{
	if (verboseLoading && consoleInstance != nullptr)
	{
		consoleInstance->Open().Output << "resman: load " << type.name << " " << resource_path.ToString() << "\n";
	}
	ResourceManagerLocation location;
	location.resourceManager = this;
	location.resourcePath = &resource_path;
	if (tryCatchFactory)
	{
		try
		{
			resource = (*type.factory)(resource_path, location);
		}
		catch (std::logic_error error)
		{
			if (consoleInstance != nullptr) consoleInstance->Open().Error << error.what() << "\n";
			resource = nullptr;
		}
	}
	else
	{
		resource = (*type.factory)(resource_path, location);
	}
}

void ResourceManager::NotifyResourceChange(const ResourcePath& path)
{
	bool fileTimestampQueried = false;
	long long fileTimestamp = 0;
	for (auto i = this->container.begin(); i!=this->container.end(); ++i)
	{
		auto& type = i->second;
		for (auto j = type.loaded.begin(); j != type.loaded.end(); ++j)
		{
			auto& resource_path = j->first;
			auto& resource_holder = j->second;
			if (resource_path == path)
			{
				// match! reload!
				if (this->timestampingService != nullptr && fileTimestampQueried == false)
				{
					fileTimestamp = this->timestampingService->GetFileTimestamp(resource_path);
					fileTimestampQueried = true;
				}
				// reload if file is newer or no information available
				if (fileTimestamp <= 0 || fileTimestamp >= resource_holder.timestamp)
				{
					ManagedResource* resource;
					CallFactory(type, resource_path, resource);
					resource_holder.basePtr = resource;
					resource_holder.timestamp = fileTimestamp;
					resource_holder.derivedPtr = nullptr;

					// inject helper variables
					if (resource != nullptr)
					{
						resource->resourceManager.resourceManager = this;
						resource->resourceManager.resourcePath = &(resource_path);
					}
				}
			}
		}
	}
}

void ResourceManager::UseTimestampingService(ITimestampingService* service)
{
	this->timestampingService = service;
}

void ResourceManager::UseConsole(Console* console)
{
	this->consoleInstance = console;
}

void ResourceManager::UseTryCatchFactory(bool tryCatch)
{
	this->tryCatchFactory = tryCatch;
}

void ResourceManager::SetVerboseLoading(bool verbose)
{
	verboseLoading = verbose;
}

ResourceManager::~ResourceManager()
{
	for (auto i = 0u; i < owned_factories_.size(); i++)
	{
		delete owned_factories_[i];
	}
}

ResourcePath::ResourcePath()
{
	data = "./";
}

ResourcePath::ResourcePath(const std::string& path)
{
	data = path; 
	Normalize();
}

ResourcePath::ResourcePath(const char* path)
{
	data = path;
	Normalize();
}

bool operator==(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return lhs.data == rhs.data;
}

bool operator!=(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return !(lhs == rhs);
}

bool operator<(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return lhs.data < rhs.data;
}

bool operator<=(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return !(rhs < lhs);
}

bool operator>(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return rhs < lhs;
}

bool operator>=(const ResourcePath& lhs, const ResourcePath& rhs)
{
	return !(lhs < rhs);
}

std::ostream& operator<<(std::ostream& os, const ResourcePath& obj)
{
	return os << (obj.IsFilePath()?"file: ":"directory: ") << obj.data;
}

ResourcePath operator+(const ResourcePath& lhs, const ResourcePath& rhs)
{
	auto path = ResourcePath(lhs); //copy
	path += rhs; //reuse += operator
	return path;
}

ResourcePath& operator+=(ResourcePath& lhs, const ResourcePath& rhs)
{
	if (!lhs.IsDirectoryPath())
	{
		throw ResourcePath::Exception("Left hand side of concatenation cannot be a file path, cannot append anything to \"" + lhs.data + "\"");
	}
	lhs.data += rhs.data;
	lhs.Normalize();
	return lhs;
}
