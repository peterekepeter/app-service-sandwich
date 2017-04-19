#pragma once
#include <string>


// a class for handling paths towards resources
class ResourcePath
{
	friend std::ostream& operator<<(std::ostream& os, const ResourcePath& obj);

public:
	// construct default path which is current folder, in relative path form
	ResourcePath();
	// can create a new path from string
	ResourcePath(const std::string& path);
	// can create a new path from const char*
	ResourcePath(const char* path);

	// concatenate 2 paths to create a third, it will be normalized
	friend ResourcePath operator +(const ResourcePath& lhs, const ResourcePath& rhs);
	// concatenate another path to the first path
	friend ResourcePath& operator +=(ResourcePath& lhs, const ResourcePath& rhs);

	friend bool operator==(const ResourcePath& lhs, const ResourcePath& rhs); //see if equal
	friend bool operator!=(const ResourcePath& lhs, const ResourcePath& rhs); //see if not equal

	ResourcePath(const ResourcePath& other); //copy
	ResourcePath(ResourcePath&& other); // move
	ResourcePath& operator=(const ResourcePath& other); //copy op
	ResourcePath& operator=(ResourcePath&& other); // move

												   // check if the path points to a file
	bool IsFilePath() const;
	// check if the path points to a directory
	bool IsDirectoryPath() const;
	// check if we have a relative path
	bool IsRelativePath() const;
	// check if we have an absolute path
	bool IsAbsolutePath() const;
	// return const char*, which can be passed to other APIs
	const char* ToCharPtr() const;
	// return a copy in std::string which can be passed (preferably moved) to other APIs
	std::string ToString() const;
	// implicit conversion operator so that you can pass ResourcePaths directly to functions that accept const char*
	operator const char*() const { return this->ToCharPtr(); }
	// convert path to a directory path
	ResourcePath ToDirectory() const;

	// relational operators are for ordering, they don't have any other meaning
	friend bool operator<(const ResourcePath& lhs, const ResourcePath& rhs);
	friend bool operator<=(const ResourcePath& lhs, const ResourcePath& rhs);
	friend bool operator>(const ResourcePath& lhs, const ResourcePath& rhs);
	friend bool operator>=(const ResourcePath& lhs, const ResourcePath& rhs);

	// hasher functor value for unordered map
	class Hasher
	{
	public:
		size_t operator()(const ResourcePath& obj) const;
	};

	// an exception type for this class only
	class Exception : public std::runtime_error
	{
	public:
		explicit Exception(const std::string& _Message) : runtime_error(_Message) { }
		explicit Exception(const char* _Message) : runtime_error(_Message) { }
	};

private:
	std::string data;

	// bring the path to a normal form, automatically called every time path changes
	// optimized, calling on already normalized paths is super fast, as there is no need for reallocation or resizing
	void Normalize();
};
