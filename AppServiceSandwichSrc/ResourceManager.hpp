#pragma once

/*
 *	Generic resource manager
 */

#include <string>
#include "ResourcePath.hpp"
#include "ITimestampingService.hpp"
#include "Console.hpp"

class ManagedResource;

class ResourceManager;

class ResourceManagerLocation;

class ResourceFactory
{
public:
	// build a new resource, resource manager is passed in case additional resources are required
	virtual ManagedResource* operator()(const ResourcePath& resourcePath, ResourceManagerLocation& resourceManager) = 0;
	virtual ~ResourceFactory(){};
};

struct ManagedResourceHolder
{
	ManagedResourceHolder();

	ManagedResourceHolder(ManagedResource* resource, void* derivedPtr);

	ManagedResourceHolder(const ManagedResourceHolder& other) = delete;

	ManagedResourceHolder(ManagedResourceHolder&& other) noexcept;

	ManagedResourceHolder& operator=(const ManagedResourceHolder& other) = delete;

	ManagedResourceHolder& operator=(ManagedResourceHolder&& other) noexcept;

	~ManagedResourceHolder();

	int reference_count;
	long long timestamp;
	ManagedResource* basePtr;
	void *derivedPtr;
};

// a managed pointer to an item of type T
template <typename T>
class ResourcePtr
{
private:
	ManagedResourceHolder* holder;

public:
	ResourcePtr() 
		: holder(nullptr)
	{

	}

	ResourcePtr(ManagedResourceHolder* holder)
		: holder(holder)
	{
		holder->reference_count++;
	};

	// copy constructor
	ResourcePtr(const ResourcePtr& other)
		: holder(other.holder) // init this->holder with other.holder
	{
		// check if is not a nulltpr
		if (holder != nullptr)
		{
			// increase reference count since we have a new pointer to the same resource
			holder->reference_count++;
		}
	}

	ResourcePtr(ResourcePtr&& other) noexcept
		: holder(other.holder)
	{
		other.holder = nullptr;
	}

	ResourcePtr& operator=(const ResourcePtr& other)
	{
		// check for self
		if (this == &other)
			return *this;

		// if has pointer, decrement is before getting the new one
		if (holder != nullptr)
			holder->reference_count--;

		// get the new one
		holder = other.holder;

		// if its not nullptr, increase its reference count
		if (holder != nullptr)
			holder->reference_count++;

		return *this;
	}

	ResourcePtr& operator=(ResourcePtr&& other) noexcept
	{
		// check for self
		if (this == &other)
			return *this;

		// steal other holder
		this->holder = other.holder;
		other.holder = nullptr;
		
		// since this gained the reference and other lost it, reference count is same

		//return 
		return *this;
	}

	// null assignment operator, causes pointer to lose reference
	std::nullptr_t operator=(std::nullptr_t null)
	{
		// if has value, decrement reference count
		if (this->holder != nullptr)
			holder->reference_count--;

		// drop the holder
		this->holder = nullptr;

		//return
		return nullptr;
	}

	// destructor
	~ResourcePtr()
	{
		if (holder != nullptr)
		{
			holder->reference_count--;
			holder = nullptr;
		}
	};

	// returns true if doesn't ponint to anything
	bool operator == (std::nullptr_t null) const
	{
		return holder == nullptr;
	}

	// returns true points to something, even if its an unloaded resoujrce
	bool operator != (std::nullptr_t null) const
	{
		return holder != nullptr;
	}

	// returns true if a resource is loaded and can be accessed
	bool IsLoaded() const
	{
		return holder != nullptr && holder->basePtr!=nullptr;
	}

	// returns true if a resource is loaded and can be accessed
	bool IsNotLoaded() const
	{
		return holder == nullptr || holder->basePtr == nullptr;
	}

	// returns true if doesn't ponint to anything
	bool IsNull() const
	{
		return holder == nullptr;
	}

	// returns true points to something, even if its an unloaded resoujrce
	bool IsNotNull() const
	{
		return holder != nullptr;
	}

	// clear this pointer, same as setting it to nullptr
	void Clear()
	{
		this->operator=(nullptr);
	}

	// arrow operator to get the pointer to T
	T* operator->() 
	{
		// TODO refactor, too many operations for pointer operator
		if (holder == nullptr)
		{
			return nullptr;
		}
		if (holder->derivedPtr == nullptr)
		{
			holder->derivedPtr = dynamic_cast<T*>(holder->basePtr);
		}
		return reinterpret_cast<T*>(holder->derivedPtr);
	};

	// defererence the pointer to get the reference
	T& operator *(){
		return *operator->();
	}
};

template <typename T> bool operator == (nullptr_t, ResourcePtr<T> ptr) {
	return ptr.operator==(nullptr);
}

template <typename T> bool operator != (nullptr_t, ResourcePtr<T> ptr) {
	return ptr.operator!=(nullptr);
}

#include <typeinfo>
#include <unordered_map>


class ResourceManagerLocation
{
	friend class ResourceManager;
private:
	ResourceManager* resourceManager = nullptr;
	const ResourcePath* resourcePath = nullptr;
public:
	template <typename T> ResourcePtr<T> Require(const ResourcePath& newLocation);
};

class ManagedResource
{
	friend class ResourceManager;
public:
	ResourceManagerLocation resourceManager;
	ManagedResource() {};
	virtual ~ManagedResource() {};
};


class ResourceManager
{
private:

	bool tryCatchFactory = false;
	bool verboseLoading = false;
	Console* consoleInstance = nullptr;
	ITimestampingService* timestampingService = nullptr;

	struct ManagedResourceType
	{
		std::string name;
		ResourceFactory* factory;
		std::unordered_map<ResourcePath, ManagedResourceHolder, ResourcePath::Hasher> loaded;

		ManagedResourceType();

		ManagedResourceType(const std::string& name, ResourceFactory& factory);
	};

	// this is where the factory is called and new resource is built
	void CallFactory(ResourceManager::ManagedResourceType& type, const ResourcePath& resource_path, ManagedResource*& resource);

	std::unordered_map<std::string, ManagedResourceType> container;
	std::vector<ResourceFactory*> owned_factories_;

public:

	// get a resource by either loading or reusing already loaded resource
	template <typename T> ResourcePtr<T> Require(const ResourcePath& key)
	{
		// get type container
		std::string name = typeid(T).name();
		if (container.find(name) == container.end()) {
			throw std::runtime_error("No factory registered for "+ name);
		}
		ManagedResourceType& type = container[name];

		// get exiting ManagedResourceHolder or create a new
		ManagedResourceHolder* finalResult;
		auto searchResult = type.loaded.find(key); // search in container by ResourcePath
		if (searchResult == type.loaded.end())
		{
			// not found, need to load new
			ManagedResource* resource_raw_ptr;

			// build
			CallFactory(type, key, resource_raw_ptr);

			// create holder object
			ManagedResourceHolder newHolder(resource_raw_ptr, nullptr);
			auto status = type.loaded.insert_or_assign(key, std::move(newHolder)); //emplace
			auto& iterator = status.first;

			// inject helper variables
			if (resource_raw_ptr != nullptr)
			{
				resource_raw_ptr->resourceManager.resourceManager = this;
				resource_raw_ptr->resourceManager.resourcePath = &(iterator->first);
			}

			// return fresly loaded 
			finalResult = &(iterator->second);
		} else
		{
			// reuse existing
			finalResult = &(searchResult->second);
		}
		
		// return the required resource
		ResourcePtr<T> pointer(finalResult);
		return pointer;
	}

	// resolve new location from current location and require a resource
	template <typename T> ResourcePtr<T> Require(const ResourcePath& currentLocation, const ResourcePath& newLocation)
	{
		ResourcePath location;
		if (newLocation.IsRelativePath())
		{
			if (currentLocation.IsDirectoryPath())
			{
				location = currentLocation + newLocation;
			}
			else
			{
				location = currentLocation.ToDirectory() + newLocation;
			}
		}
		else
		{
			location = newLocation;
		}
		return this->Require<T>(location);
	}

	// register a factory instance that can build resources of type T
	template <typename T> void RegisterFactory(ResourceFactory& instance) {
		std::string name = typeid(T).name();
		ManagedResourceType managedType(name, instance);
		container.insert_or_assign(name, std::move(managedType));
	}

	// register a factory type that can build resources of type T, this calls the default constructor of T
	template <typename T, typename TFactory> void RegisterFactory()
	{
		auto factory = new TFactory();
		owned_factories_.push_back(factory);
		RegisterFactory<T>(*factory);
	}

	//notify the manager that a resource at a given path has changed, and need reloading
	void NotifyResourceChange(const ResourcePath& path);

	void UseTimestampingService(ITimestampingService* service);

	void UseConsole(Console* console);

	void UseTryCatchFactory(bool tryCatch);

	void SetVerboseLoading(bool verbose);

	~ResourceManager();
private:
	
};

template<typename T>
inline ResourcePtr<T> ResourceManagerLocation::Require(const ResourcePath& newLocation)
{
	return resourceManager->Require<T>(*resourcePath, newLocation);
}


/*
	idea box:

	resource identification by path name, as implemented by ResourcePath
	but 2 issues
		1. paths should be absolute? (c:/users/stuff.txt) (absolute path required)
		2. paths relative to project? 
		3. everything is relative (assumes that current directory doesn't change, if it does bad things happen)

	resource loading strategy: (can be all of these)
		1. factory functor for each resource type (or lambda, or class, or all of these)
		2. resource can load itself

	ket resource injections strategia: (choose one)
		1. pointer indirection, elony: resourcemanager runtime tudja injectelni az updatolt resourceot
		2. notify dependant object and reinitialize it (ResourceConsumer interface)
		3. resource pointer injection, resource pointers are injected into dependant classes 
			- in danger of memory leak

	extra ideas:
		- temporary default resources while loading
		- loading progress support

	garbage collection: reference counting, manual trigger
*/