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
	lua::root.set_protected_calls(true);
	// lua::do_chunk("my_scope.print = print");
	lua::do_chunk(
"function world(name)\
	print(\"Hello \"..name..\"!\")\
end", lua::root["my_scope"]);


	auto result = lua::root["my_scope"]["world"]("Joe");
	if(!result) {
		std::cout << result.error().stack() << std::endl;
	}

	return 0;
}