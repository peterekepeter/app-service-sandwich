#pragma once
#include <vector>
#include <sstream>
#include <mutex>

class IConsoleDriver
{
public:
	virtual ~IConsoleDriver()
	{
	}

	virtual void WriteOutput(const std::string& buffer) = 0;
	virtual void WriteError(const std::string& buffer) = 0;
};

// represents a console with standard, error and input streams
// TODO: input
class Console
{
	friend class Transaction;
	std::mutex lock;
	IConsoleDriver* driver = nullptr;
	std::vector<std::string> outputBuffer;
	std::vector<std::string> errorBuffer;
	size_t progressTag = 0;
	void FlushDriverlessBuffers();
public:

	Console();

	Console(IConsoleDriver* driver);

	void UseDriver(IConsoleDriver* driver);

	class Transaction
	{
	private:
		Console *stream;

		size_t progressTag;

	public:
		Transaction(const Transaction& other) = delete;

		Transaction(Transaction&& other) noexcept;

		Transaction& operator=(const Transaction& other) = delete;

		Transaction& operator=(Transaction&& other) noexcept;

	private:
		void Write();

		void Clear();
	public:

		explicit Transaction(Console* stream);

		explicit Transaction(Console* stream, size_t progressTag);

		std::ostringstream Output;
		std::ostringstream Error;

		void Commit();

		void Rollback();

		~Transaction();
	};

	// if two subsequent progress prints share the same tag
	// they will override each other, useful for progress bars
	Transaction ProgressPrinter(const void* tag);

	// Creates a new printer to print with 
	// (deprecated) use CreatePrinter() insatead
	Transaction Open();

	// Creates a new printer to print with
	Transaction Printer();

	Transaction operator ()();

	static void SetDefaultInstance(Console* instance);
	static Transaction Default();
};
