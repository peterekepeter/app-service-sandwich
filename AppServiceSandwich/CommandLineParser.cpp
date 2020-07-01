#include "CommandLineParser.hpp"

static void CheckBooleanRef(bool& valueRef);

void CommandLineParser::Parse(
	char** argv,
	int argc)
{
	Parse((const char** )argv, argc);
}

void CommandLineParser::Parse(
	const char** argv, 
	int argc)
{
	if (resultExecutableName != nullptr) {
		*resultExecutableName = argv[0];
	}
	for (int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		const char* next = nullptr;
		if (i + 1 < argc) {
			next = argv[i + 1];
		}
		{
			auto match = boolOptions.find(arg);
			if (match != boolOptions.end()) {
				*(match->second) = true;
				continue;
			}
		}
		{
			auto intMatch = intOptions.find(arg);
			if (intMatch != intOptions.end()) {
				if (next != nullptr) {
					*(intMatch->second) = atoi(next);
					i++;
					continue;
				}
				else {
					throw std::runtime_error("missing expected integer");
				}
			}
		}
		{
			auto strMatch = stringOptions.find(arg);
			if (strMatch != stringOptions.end()) {
				if (next != nullptr) {
					*(strMatch->second) = next;
					i++;
					continue;
				}
				else {
					throw std::runtime_error("missing expected string");
				}
			}
		}
		throw std::runtime_error(std::string("option \"") + arg + "\" not found!");
	}
}

CommandLineParser& CommandLineParser::ExecutableName(
	std::string& output)
{ 
	resultExecutableName = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias, 
	bool& output)
{
	CheckBooleanRef(output);
	boolOptions[alias] = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias,
	const std::string& alias2,
	bool& output)
{
	CheckBooleanRef(output);
	boolOptions[alias] = &output;
	boolOptions[alias2] = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias,
	int& output)
{
	intOptions[alias] = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias,
	const std::string& alias2,
	int& output)
{
	intOptions[alias] = &output;
	intOptions[alias2] = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias,
	std::string& output)
{
	stringOptions[alias] = &output;
	return *this;
}

CommandLineParser& CommandLineParser::Option(
	const std::string& alias,
	const std::string& alias2,
	std::string& output)
{
	stringOptions[alias] = &output;
	stringOptions[alias2] = &output;
	return *this;
}




static void CheckBooleanRef(bool& valueRef) {
	if (valueRef == true) {
		throw std::invalid_argument(
			"referenced boolean should be false");
	}
}