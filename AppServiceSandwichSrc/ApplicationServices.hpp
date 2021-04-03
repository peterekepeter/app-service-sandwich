#pragma once
#include "Console.hpp"
#include "DependencyManager.hpp"
#include "ResourceManager.hpp"

// a convenience class that groups together the resource manager, dependency manager and console
// contains abstract services useful for many applications
class ApplicationServices
{
public:

	ApplicationServices()
	{
		Console::SetDefaultInstance(&console);
		dependency.For<ResourceManager>().UseSharedInstance(&resource);
		dependency.For<Console>().UseSharedInstance(&console);
		resource.UseConsole(&console);
		resource.UseTryCatchFactory(true);
	}

	ApplicationServices(const ApplicationServices& other) = delete;
	ApplicationServices(ApplicationServices&& other) noexcept = delete;
	ApplicationServices& operator=(const ApplicationServices& other) = delete;
	ApplicationServices& operator=(ApplicationServices&& other) noexcept = delete;

	ResourceManager resource;
	DependencyManager dependency;
	Console console;
};
