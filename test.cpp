#include "lua/lua.hpp"

int main(int argc, char const *argv[])
{
	lua::root["my_scope"] = {
		{"my_string", "some text"},
		{"my_table", {
			{"nested_thing", "woohoo!"}
		}},
		{"my_data", 0.9234}
	};

	return 0;
}