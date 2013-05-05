#include "lua/lua.hpp"
#include <type_traits>
#include <iostream>

int main(int argc, char const *argv[])
{
	try {
		lua::root["my_scope"] = {
			{"my_string", "some text"},
			{"my_table", {
				{"nested_thing", "woohoo!"}
			}},
			{"my_data", 0.9234},
			{"my_bool", true},
			{"my_chunk", lua::chunk("local world = ...; print(\"Hello \"..world..\"!\")")}
		};

		lua::root["my_scope"]["my_chunk"]("Matt");
	} catch (lua::exception e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}