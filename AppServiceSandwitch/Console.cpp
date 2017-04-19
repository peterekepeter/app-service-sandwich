#include "Console.hpp"

static Console* defaultInstance = nullptr;

void Console::SetDefaultInstance(Console* instance)
{
	defaultInstance = instance;
}

Console::Transaction Console::Default()
{
	return defaultInstance->Open();
}

void Console::FlushDriverlessBuffers()
{
}

Console::Console()
{
}

Console::Console(IConsoleDriver* driver)
{
	UseDriver(driver);
}

void Console::UseDriver(IConsoleDriver* driver)
{
	lock.lock();
	this->driver = driver;
	// flush those buffers
	if (driver != nullptr)
	{
		for (auto& output : this->outputBuffer)
		{
			driver->WriteOutput(output);
		}
		for (auto& error : this->errorBuffer)
		{
			driver->WriteError(error);
		}
		this->outputBuffer.clear();
		this->errorBuffer.clear();
	}
	lock.unlock();
}

Console::Transaction::Transaction(Transaction&& other) noexcept: stream(other.stream),
                                                                 Output(std::move(other.Output)),
                                                                 Error(std::move(other.Error))
{
}

Console::Transaction& Console::Transaction::operator=(Transaction&& other) noexcept
{
	if (this == &other)
		return *this;
	stream = other.stream;
	Output = std::move(other.Output);
	Error = std::move(other.Error);
	return *this;
}

void Console::Transaction::Write()
{
	stream->lock.lock();
	if (stream->driver)
	{
		stream->driver->WriteOutput(Output.str());
		stream->driver->WriteError(Error.str());
	}
	else
	{
		stream->outputBuffer.push_back(Output.str());
		stream->errorBuffer.push_back(Error.str());
	}
	stream->lock.unlock();
}

void Console::Transaction::Clear()
{
	Output.clear();
	Error.clear();
}

Console::Transaction::Transaction(Console* stream): stream(stream)
{
}

void Console::Transaction::Commit()
{
	Write();
	Clear();
}

void Console::Transaction::Rollback()
{
	Clear();
}

Console::Transaction::~Transaction()
{
	Commit();
}

Console::Transaction Console::Open()
{
	return Transaction(this);
}

Console::Transaction Console::operator()()
{
	return Open();
}
