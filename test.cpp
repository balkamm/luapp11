#include "lua/lua.hpp"
#include <type_traits>
#include <iostream>

int main(int argc, char const *argv[])
{
	lua::root["my_scope"] = {
		{"my_string", "some text"},
		{"my_table", {
			{"nested_thing", "woohoo!"}
		}},
		{"my_data", 0.9234},
		{"my_bool", true},
		{"my_int", 1}
	};
	lua::do_chunk("my_scope.print = print");
	lua::do_chunk(
"function world(name)\
	print(\"Hello \"..name..\"!\")\
end", lua::root["my_scope"]);


	lua::root["my_scope"]["world"]("Joe");

	return 0;
}