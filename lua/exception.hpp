#pragma once

#include <exception>
#include <string>

namespace lua
{
class exception : std::exception
{
public:
	exception(std::string what)
		: what_(what)
	{}

	const char* what() const throw() override {
		return what_.c_str();
	}

private:
	std::string what_;
};
}