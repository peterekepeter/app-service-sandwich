
#include "ChangeInformation.hpp"
#include <ostream>
#include <string>

std::ostream& operator<<(std::ostream& os, const ChangeInformation& obj)
{
	return os
		<< "action: " << obj.action
		<< ", filename: " << obj.filename;
}

ChangeInformation::ChangeInformation(const ChangeInformation& other) : action(other.action),
filename(other.filename)
{
}

ChangeInformation::ChangeInformation(ChangeInformation&& other) : action(other.action),
filename(std::move(other.filename))
{
}

ChangeInformation& ChangeInformation::operator=(const ChangeInformation& other)
{
	if (this == &other)
		return *this;
	action = other.action;
	filename = other.filename;
	return *this;
}

ChangeInformation& ChangeInformation::operator=(ChangeInformation&& other)
{
	if (this == &other)
		return *this;
	action = other.action;
	filename = std::move(other.filename);
	return *this;
}

std::ostream& operator<<(std::ostream& os, const ChangeInformation::Action& obj)
{
	const char* s = "Unknown";
	switch (obj)
	{
	case ChangeInformation::Action::Added: s = "Added";
		break;
	case ChangeInformation::Action::Modified: s = "Modified";
		break;
	case ChangeInformation::Action::Removed: s = "Removed";
		break;
	case ChangeInformation::Action::RenamedOldName: s = "RenamedOldName";
		break;
	case ChangeInformation::Action::RenamedNewName: s = "RenamedNewName";
		break;
	}
	return os << s;
}

ChangeInformation::ChangeInformation(::ChangeInformation::Action action, const std::string& filename) : action(action),
filename(filename)
{
}
