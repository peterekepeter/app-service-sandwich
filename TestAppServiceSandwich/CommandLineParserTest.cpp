#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../AppServiceSandwichSrc/CommandLineParser.hpp"

namespace Test
{
	TEST_CLASS(CommandLineParserTest)
	{
	public:
		TEST_METHOD(DefaultConstructible)
		{
			CommandLineParser parser;
		}

		TEST_METHOD(ParsesArgumentList)
		{
			CommandLineParser parser;
			const char* args[] = { "node" };
			parser.Parse(args, sizeof(args) / sizeof(const char*));
		}

		TEST_METHOD(ParsesExecutableName)
		{
			std::string parsed;
			CommandLineParser parser;
			parser.ExecutableName(parsed);
			const char* args[] = { "node" };
			parser.Parse(args, sizeof(args) / sizeof(const char*));
			Assert::AreEqual(parsed, std::string(args[0]));
		}

		TEST_METHOD(ParsesOptionFlag)
		{
			const char* argv[] = { "node", "-v" };
			int argc = sizeof(argv) / sizeof(const char*);
			bool printVersion = false;
			CommandLineParser()
				.Option("-v", printVersion)
				.Parse(argv, argc);
			Assert::IsTrue(printVersion);
		}

		TEST_METHOD(ParsesOptionFlagAlias)
		{
			const char* argv[] = { "node", "--version" };
			int argc = sizeof(argv) / sizeof(const char*);
			bool printVersion = false;
			CommandLineParser()
				.Option("-v", "--version", printVersion)
				.Parse(argv, argc);
			Assert::IsTrue(printVersion);
		}

		TEST_METHOD(ThrowsErrorIfBooleanOutputIsTrue)
		{
			Assert::ExpectException<std::invalid_argument, void>([]{
				bool printVersion = true;
				CommandLineParser()
					.Option("-v", "--version", printVersion);
			});
			Assert::ExpectException<std::invalid_argument, void>([] {
				bool printVersion = true;
				CommandLineParser()
					.Option("-v", printVersion);
			});
		}

		TEST_METHOD(ParsesIntegerOption)
		{
			const char* argv[] = { "prog", "-i", "4" };
			int argc = sizeof(argv) / sizeof(const char*);
			int id = 0;
			CommandLineParser()
				.Option("-i", "--id", id)
				.Parse(argv, argc);
			Assert::AreEqual(id, 4);

			id = 0;
			CommandLineParser()
				.Option("-i", id)
				.Parse(argv, argc);
			Assert::AreEqual(id, 4);
		}

		TEST_METHOD(ParsesStringOptionSecondAlias) 
		{
			const char* argv[] = { "prog", "--mode", "test" };
			int argc = sizeof(argv) / sizeof(const char*);
			std::string mode = "n/a";
			CommandLineParser()
				.Option("-m", "--mode", mode)
				.Parse(argv, argc);
			Assert::AreEqual(mode, std::string("test"));
		}

		TEST_METHOD(ParsesStringOptionFirstAlias)
		{
			const char* argv[] = { "prog", "-m", "test" };
			int argc = sizeof(argv) / sizeof(const char*);
			std::string mode = "n/a";
			CommandLineParser()
				.Option("-m", "--mode", mode)
				.Parse(argv, argc);
			Assert::AreEqual(mode, std::string("test"));
		}

		TEST_METHOD(ParsesStringOption)
		{
			const char* argv[] = { "prog", "-m", "test" };
			int argc = sizeof(argv) / sizeof(const char*);
			std::string mode = "n/a";
			CommandLineParser()
				.Option("-m", mode)
				.Parse(argv, argc);
			Assert::AreEqual(mode, std::string("test"));
		}

		TEST_METHOD(ThrowsErrorIfStringOptionArgumentIsMissing)
		{
			Assert::ExpectException<std::runtime_error, void>([] {
				const char* argv[] = { "prog", "-m" };
				int argc = sizeof(argv) / sizeof(const char*);
				std::string mode = "n/a";
				CommandLineParser()
					.Option("-m", mode)
					.Parse(argv, argc);
			});
		}

		TEST_METHOD(ThrowsErrorIfIntegerOptionArgumentIsMissing)
		{
			Assert::ExpectException<std::runtime_error, void>([] {
				int id = 0;
				const char* argv[] = { "prog", "-i" };
				int argc = sizeof(argv) / sizeof(const char*);
				CommandLineParser()
					.Option("-i", "--id", id)
					.Parse(argv, argc);
			});
		}

		TEST_METHOD(ThrowsErrorIfOptionNotFound)
		{
			Assert::ExpectException<std::runtime_error, void>([] {
				int id = 0;
				const char* argv[] = { "prog", "-n" };
				int argc = sizeof(argv) / sizeof(const char*);
				CommandLineParser()
					.Parse(argv, argc);
			});
		}

	};
}