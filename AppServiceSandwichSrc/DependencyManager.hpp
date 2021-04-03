#pragma once
#include <functional>
#include <unordered_map>
#include <type_traits>


class DependencyManager
{
public:

	template <typename T> class ExpressionContext
	{
		friend class DependencyManager;
		std::function<T*(void)> factory;
		T* instance;
		bool owned;
		DependencyManager* manager;

		void Cleanup();
		ExpressionContext(DependencyManager*manager);

	public:

		// Use the constructor with single argument of type D0
		template <typename D0> void UseConstructor();

		// Use the constructor with two arguments of type D0, D1
		template <typename D0, typename D1> void UseConstructor();

		// map a required type to other type, returns the expression context for the new other type
		template <typename D> ExpressionContext<D>& UseType();

		// register a type to use for another type
		void UseDefaultConstructor();

		// use factory
		void UseFactory(std::function<T*(void)> paramFactory);

		// use intance and move it permanently here
		void UseInstanceTransferOwnership(T* newInstance);

		// use shared instance
		[[deprecated("use UseSharedInstance instead")]]
		void UseInstance(T* newInstance);

		// use shared instance
		void UseSharedInstance(T* newInstance);

		~ExpressionContext();

	};

	// Get the instance configured for type T. The instance may not be instantiated.
	template <typename T> T* GetInstance();

	// Get the context for a specific type so that you can configure it
	template <typename T> ExpressionContext<T>& For();

	~DependencyManager()
	{
		for (auto ptr: container)
		{
			delete ptr.second;
			ptr.second = nullptr;
		}
	}
private:

	std::unordered_map<std::string, void*> container;

	template <typename T> 
	typename std::enable_if<
		std::is_constructible<T, DependencyManager*>::value, T*
	>::type TryCreateAutoInstance() 
	{
		return new T(this);
	}

	template <typename T> 
	typename std::enable_if<
		std::is_default_constructible<T>::value &&
		!std::is_constructible<T, DependencyManager*>::value, T*
	>::type TryCreateAutoInstance() 
	{
		return new T();
	}

	template <typename T>
	typename std::enable_if<
		!std::is_default_constructible<T>::value && 
		!std::is_constructible<T, DependencyManager*>::value, T*
	>::type TryCreateAutoInstance()
	{
		return nullptr;
	}

};

template <typename T> DependencyManager::ExpressionContext<T>& DependencyManager::For()
{
	ExpressionContext<T>* ptr;
	// get type container
	std::string name = typeid(T).name();
	auto search = container.find(name);
	if (search == container.end()) {
		ptr = new DependencyManager::ExpressionContext<T>(this);
		container[name] = ptr;
	}
	else
	{
		ptr = reinterpret_cast<DependencyManager::ExpressionContext<T>*>(search->second);
	}
	return *ptr;
}

template <typename T> T* DependencyManager::GetInstance()
{
	ExpressionContext<T>& context = For<T>();
	if (context.instance == nullptr)
	{
		if (context.factory == nullptr)
		{
			context.instance = TryCreateAutoInstance<T>();
			if (context.instance == nullptr) {
				// if you end up here with debugger, you need to add a registration in 
				// DependencyManager for your type
				throw std::runtime_error(
					std::string("Dependency is not configured for \"")
					+ typeid(T).name() + "\", configuration required.");
			}
		}
		else 
		{
			// build the instance
			context.instance = context.factory();
		}
		// check if returned nullptr
		if (context.instance == nullptr)
		{
			throw std::runtime_error(std::string("Factory returned nullptr for ") + typeid(T).name() + ".");
		}
	}
	return context.instance;
}

template <typename T> 
void DependencyManager::ExpressionContext<T>::Cleanup()
{
	if (owned)
	{
		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	}
	else
	{
		instance = nullptr;
	}
	owned = false;
}

template<typename T>
inline DependencyManager::ExpressionContext<T>::ExpressionContext(DependencyManager * manager) : manager(manager)
{
	instance = nullptr;
	owned = false;
	factory = nullptr;
}

// register a type to use for another type
template<typename T>
inline void DependencyManager::ExpressionContext<T>::UseDefaultConstructor()
{
	Cleanup();
	owned = true;
	factory = []() { return new T; };
}

// use factory
template<typename T>
inline void DependencyManager::ExpressionContext<T>::UseFactory(std::function<T*(void)> paramFactory)
{
	Cleanup();
	owned = true;
	factory = paramFactory;
}

// use intance and move it permanently here
template<typename T>
inline void DependencyManager::ExpressionContext<T>::UseInstanceTransferOwnership(T * newInstance)
{
	Cleanup();
	owned = true;
	instance = newInstance;
}

// use shared instance
template<typename T>
inline void DependencyManager::ExpressionContext<T>::UseInstance(T * newInstance)
{
	UseSharedInstance(newInstance);
}

// use shared instance
template<typename T>
inline void DependencyManager::ExpressionContext<T>::UseSharedInstance(T* newInstance)
{
	Cleanup();
	owned = false;
	instance = newInstance;
}


template<typename T>
inline DependencyManager::ExpressionContext<T>::~ExpressionContext()
{
	Cleanup();
	manager = nullptr;
}

// Use the constructor with single argument of type D0
template<typename T>
template<typename D0>
inline void DependencyManager::ExpressionContext<T>::UseConstructor()
{
	Cleanup();
	owned = true;
	factory = [this] { return new T(this->manager->GetInstance<D0>()); };
}

// Use the constructor with two arguments of type D0, D1
template<typename T>
template<typename D0, typename D1>
inline void DependencyManager::ExpressionContext<T>::UseConstructor()
{
	Cleanup();
	owned = true;
	factory = [this] { return new T(this->manager->GetInstance<D0>(), this->manager->GetInstance<D1>()); };
}

// map a required type to other type, returns the expression context for the new other type
template<typename T>
template<typename D>
inline DependencyManager::ExpressionContext<D>& DependencyManager::ExpressionContext<T>::UseType()
{
	Cleanup();
	owned = false;
	factory = [this]() { return this->manager->GetInstance<D>(); };
	return manager->For<D>();
}
