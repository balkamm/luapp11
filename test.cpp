#include "lua/lua.hpp"

#include <iostream>

int main(int argc, char const *argv[])
{
	lua::root["my_scope"] = {
		{"my_string", "some text"},
		{"my_table", {
			{"nested_thing", "woohoo!"}
		}},
		{"my_data", 0.9234}
	};

	std::cout << lua::root["my_scope"]["my_data"].as<std::string>("Whoops!") << std::endl;
	std::cout << lua::root["my_scope"]["my_string"].as<std::string>("Whoops!") << std::endl;

	return 0;
}