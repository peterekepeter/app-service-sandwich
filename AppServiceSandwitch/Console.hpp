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
	void FlushDriverlessBuffers();
public:

	Console();

	Console(IConsoleDriver* driver);

	void UseDriver(IConsoleDriver* driver);

	class Transaction
	{
	private:
		Console *stream;

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

		std::ostringstream Output;
		std::ostringstream Error;

		void Commit();

		void Rollback();

		~Transaction();
	};

	Transaction Open();

	Transaction operator ()();

	static void SetDefaultInstance(Console* instance);
	static Transaction Default();
};
