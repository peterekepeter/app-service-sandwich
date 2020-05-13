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
	bool clearLine = false;
	bool insertNewline = false;
	if (progressTag != 0 && stream->progressTag == progressTag) {
		clearLine = true;
	}
	else if (stream->progressTag != 0) {
		insertNewline = true;
	}

	auto outStr = Output.str();
	auto errStr = Error.str();
	if (outStr.length() == 0 && errStr.length() == 0) {
		return; // nothing to print
	}

	stream->lock.lock();
	if (stream->driver)
	{
		auto driver = stream->driver;
		if (outStr.length() > 0) {
			if (clearLine) {
				driver->WriteOutput("\r");
			}
			else if (insertNewline) {
				driver->WriteOutput("\n");
			}
			driver->WriteOutput(outStr);
		}
		if (errStr.length() > 0) {
			if (clearLine) {
				driver->WriteError("\r");
			} else if (insertNewline) {
				driver->WriteOutput("\n");
			}
			driver->WriteError(errStr);
		}
	}
	else
	{
		if (outStr.length() > 0) {
			auto& buffer = stream->outputBuffer;
			auto& str = outStr; 
			if (clearLine && buffer.size() > 0)
			{
				buffer[buffer.size() - 1] = str;
			}
			else {
				if (insertNewline) {
					buffer.push_back("\n");
				}
				buffer.push_back(str);
			}
		}
		if (errStr.length() > 0) {
			auto& buffer = stream->errorBuffer;
			auto& str = errStr;
			if (clearLine && buffer.size() > 0)
			{
				buffer[buffer.size() - 1] = str;
			}
			else {
				if (insertNewline) {
					buffer.push_back("\n");
				}
				buffer.push_back(str);
			}
		}
	}
	stream->progressTag = progressTag;
	stream->lock.unlock();
}

void Console::Transaction::Clear()
{
	Output.clear();
	Error.clear();
}

Console::Transaction::Transaction(Console* stream)
	: stream(stream)
	, progressTag(0)
{
}

Console::Transaction::Transaction(Console* stream, size_t progressTag)
	: stream(stream)
	, progressTag(progressTag)
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

Console::Transaction Console::ProgressPrinter(const void* tag)
{
	return Transaction(this, reinterpret_cast<size_t>(tag));
}

Console::Transaction Console::Open()
{
	return Transaction(this);
}

Console::Transaction Console::Printer()
{
	return Open();
}

Console::Transaction Console::operator()()
{
	return Open();
}
