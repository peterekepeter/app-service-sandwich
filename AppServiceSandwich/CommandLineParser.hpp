#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

class CommandLineParser
{
public:
	// needs tokenizer
	/*void Parse(const std::string& str);
	void Parse(const char* str);*/

	void Parse(
		char** argv, 
		int argc);

	void Parse(
		const char** argv,
		int argc);

	CommandLineParser& ExecutableName(
		std::string& output); 

	CommandLineParser& Option(
		const std::string& alias, 
		bool& output);

	CommandLineParser& Option(
		const std::string& alias,
		std::string& output);

	CommandLineParser& Option(
		const std::string& alias,
		int& output);

	CommandLineParser& Option(
		const std::string& alias, 
		const std::string& alias2, 
		bool& output);

	CommandLineParser& Option(
		const std::string& alias,
		const std::string& alias2,
		int& output);

	CommandLineParser& Option(
		const std::string& alias,
		const std::string& alias2,
		std::string& output);


private:
	std::string* resultExecutableName = nullptr;

	std::map<std::string, bool*> boolOptions;
	std::map<std::string, int*> intOptions;
	std::map<std::string, std::string*> stringOptions;
};