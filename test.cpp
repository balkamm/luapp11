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

	std::cout << lua::root["my_scope"]["my_data"].get<double>() << std::endl;

	return 0;
}