#pragma once

#include <exception>
#include <string>

namespace lua
{
class Exception : std::exception
{
public:
	Exception(std::string what)
		: what_(what)
	{}

	const char* what() const throw() override {
		return what_.c_str();
	}

private:
	std::string what_;
};
}