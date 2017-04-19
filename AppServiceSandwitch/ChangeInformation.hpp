#pragma once
#include <string>

// information about a file mondification
class ChangeInformation
{
	friend std::ostream& operator<<(std::ostream& os, const ChangeInformation& obj);

public:
	ChangeInformation(const ChangeInformation& other);

	ChangeInformation(ChangeInformation&& other);

	ChangeInformation& operator=(const ChangeInformation& other);

	ChangeInformation& operator=(ChangeInformation&& other);

	enum class Action
	{
		None = 0,
		Added = 1,
		Removed = 2,
		Modified = 3,
		RenamedOldName = 4,
		RenamedNewName = 5
	};

	friend std::ostream& operator<<(std::ostream& os, const Action& obj);

	ChangeInformation(::ChangeInformation::Action action, const std::string& filename);

	Action action;
	std::string filename;
};
