#pragma once
#include <functional>
#include <unordered_map>


class DependencyManager
{
	std::unordered_map<std::string, void*> container;

public:
	
	template <typename T> class ExpressionContext
	{
		friend class DependencyManager;
		std::function<T*(void)> factory;
		T* instance;
		bool owned;
		DependencyManager* manager;

		void Cleanup()
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

		ExpressionContext(DependencyManager*manager) : manager(manager)
		{
			instance = nullptr;
			owned = false;
			factory = nullptr;
		}

	public:

		// Use the constructor with single argument of type D0
		template <typename D0> void UseConstructor()
		{
			Cleanup();
			owned = true;
			factory = [this]() { return new T(this->manager->GetInstance<D0>()); };
		}

		// Use the constructor with two arguments of type D0, D1
		template <typename D0, typename D1> void UseConstructor()
		{
			Cleanup();
			owned = true;
			factory = [this]() { return new T(this->manager->GetInstance<D0>(), this->manager->GetInstance<D1>()); };
		}

		// map a required type to other type, returns the expression context for the new other type
		template <typename D> ExpressionContext<D>& UseType()
		{
			Cleanup();
			owned = false;
			factory = [this]() { return this->manager->GetInstance<D>(); };
			return manager->For<D>();
		}

		// register a type to use for another type
		void UseDefaultConstructor()
		{
			Cleanup();
			owned = true;
			factory = []() { return new T; };
		}

		// use factory
		void UseFactory(std::function<T*(void)> paramFactory)
		{
			Cleanup();
			owned = true;
			factory = paramFactory;
		}
		
		// use intance and move it permanently here
		void UseInstanceTransferOwnership(T* newInstance)
		{
			Cleanup();
			owned = true;
			instance = newInstance;
		}

		// use shared instance
		void UseInstance(T* newInstance)
		{
			Cleanup();
			owned = false;
			instance = newInstance;
		}

		~ExpressionContext()
		{
			Cleanup();
			manager = nullptr;
		}

		
	};

	// Get the context for a specific type so that you can configure it
	template <typename T> ExpressionContext<T>& For()
	{
		ExpressionContext<T>* ptr;
		// get type container
		std::string name = typeid(T).name();
		auto search = container.find(name);
		if (search == container.end()) {
			ptr = new ExpressionContext<T>(this);
			container[name] = ptr;
		}
		else
		{
			ptr = reinterpret_cast<ExpressionContext<T>*>(search->second);
		}
		return *ptr;
	}
	
	// Get the instance configured for type T. The instance may not be instantiated.
	template <typename T> T* GetInstance()
	{
		ExpressionContext<T>& context = For<T>();
		if (context.instance == nullptr)
		{
			if (context.factory == nullptr)
			{
				throw std::runtime_error(std::string("Dependency is not configured for ") + typeid(T).name() + ".");
			}
			else
			{
				// build the instance
				context.instance = context.factory();
				// check if returned nullptr
				if (context.instance == nullptr)
				{
					throw std::runtime_error(std::string("Factory returned nullptr for ") + typeid(T).name() + ".");
				}
			}
		}
		return context.instance;
	}

	~DependencyManager()
	{
		for (auto ptr: container)
		{
			delete ptr.second;
			ptr.second = nullptr;
		}
	}
};
