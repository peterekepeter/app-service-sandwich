
//for not windows you might want to define these
//#define RESOURCE_PATH_IS_CASE_SENSITIVE
//#define RESOURCE_PATH_NO_DRIVE_LETTER_SUPPORT

#include "ResourcePath.hpp"

ResourcePath::ResourcePath(const ResourcePath& other) : data(other.data)
{
}

ResourcePath::ResourcePath(ResourcePath&& other) : data(std::move(other.data))
{
}

ResourcePath& ResourcePath::operator=(const ResourcePath& other)
{
	if (this == &other)
		return *this;
	data = other.data;
	return *this;
}

ResourcePath& ResourcePath::operator=(ResourcePath&& other)
{
	if (this == &other)
		return *this;
	data = std::move(other.data);
	return *this;
}

bool ResourcePath::IsFilePath() const
{
	return !IsDirectoryPath();
}

bool ResourcePath::IsDirectoryPath() const
{
	return data[data.size() - 1] == '/';
}

bool ResourcePath::IsRelativePath() const
{
	bool absolutePath = false;
	if (data.size()>0)
	{
		if (data[0] == '/') {
			absolutePath = true;
		}
#ifndef RESOURCE_PATH_NO_DRIVE_LETTER_SUPPORT
		else if (isalpha(data[0]) && data[1] == ':' && data[2] == '/')
		{
			absolutePath = true;
		}
#endif
	}
	return !absolutePath;
}

bool ResourcePath::IsAbsolutePath() const
{
	return !IsRelativePath();
}

const char* ResourcePath::ToCharPtr() const
{
	return data.c_str();
}

std::string ResourcePath::ToString() const
{
	return data;
}

ResourcePath ResourcePath::ToDirectory() const
{
	if (this->IsDirectoryPath())
	{
		return *this;
	}
	for (int i = data.size() - 1; i >= 0; i--)
	{
		char c = data[i];
		if (c == '/')
		{
			return ResourcePath(data.substr(0, i + 1));
		}
	}
	return ResourcePath(); // current dir
}

#include <vector>

size_t ResourcePath::Hasher::operator()(const ResourcePath& obj) const
{
	return std::hash<std::string>()(obj.data);
}

// implemented as a simple state machine parser, since called regularly, optimised
void ResourcePath::Normalize()
{
	// state variables
	int src, dest;
	src = dest = data.size() - 1;
	bool pathSep = false;
	bool needToCompletePath = true;
	int dotCount = 0;
	int upLevel = 0;
	char c = 0; // current char
				// parse the path
	while (src >= 0)
	{
		c = data[src--];
		switch (c)
		{
		case '/':
		case '\\':
			pathSep = true;
			if (dotCount > 0)
			{
				if (dotCount >= 2)
				{
					// adjustment for proper skipping after ../ during loop
					upLevel = (upLevel == 0 ? 2 : upLevel + 1);
				}
				dotCount = 0;
			}
			break;
		case '.':
			if (pathSep)
			{
				dotCount++;
				break;
			}
			// else pass through
		default:
			if (upLevel > 0)
			{
				if (pathSep)
				{
					upLevel--;
					if (upLevel != 0)
					{
						pathSep = false;
					}
				}
			}

			if (upLevel == 0)
			{
				if (pathSep)
				{
					data[dest--] = '/';
					pathSep = false;
				}
#ifndef RESOURCE_PATH_IS_CASE_SENSITIVE
				c = tolower(c);
#endif
				data[dest--] = c;
			}
		}
	}
	// adjust back from parser loop to avoid +1 concatenation of ../
	if (upLevel > 0)
	{
		upLevel--;
	}
	// terminate the path based on current state
	if (dotCount >= 2)
	{
		upLevel++;
	}
#ifndef RESOURCE_PATH_NO_DRIVE_LETTER_SUPPORT
	if (isalpha(c))
	{
		if (static_cast<unsigned>(dest + 3)<data.size() && data[dest + 2] == ':' && data[dest + 3] == '/')
		{
			// we have a drive letter path
			if (upLevel>0)
			{
				throw ResourcePath::Exception("Invalid path, trying to go to upper parent folder from root folder.");
			}
			needToCompletePath = false; //it's done then
		}

	}
#endif
	if (needToCompletePath)
	{
		if (c == '/' || c == '\\')
		{
			if (upLevel > 0)
			{
				throw ResourcePath::Exception("Invalid path, trying to go to upper parent folder from root folder.");
			}
			data[dest--] = '/'; //relative to root
		}
		else
		{
			if (upLevel>0)
			{
				while (upLevel--)
				{
					if (dest < 2)
					{
						// need some allocation
						data = "../" + data.substr(dest + 1);
						dest = -1;
					}
					else
					{
						data[dest--] = '/';
						data[dest--] = '.';
						data[dest--] = '.';
					}
				}
			}
			else
			{
				if (dest < 1)
				{
					// need some allocation
					data = "./" + data.substr(dest + 1);
					dest = -1;
				}
				else
				{
					data[dest--] = '/';
					data[dest--] = '.';
				}
			}
		}
	}
	// adjust the string data
	if (dest != -1)
	{
		// need adjustment
		data = data.substr(dest + 1);
	}
}