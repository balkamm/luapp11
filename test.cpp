#include "lua/state.hpp"
#include "lua/value.hpp"

int main(int argc, char const *argv[])
{
	lua::State state;
	state.Env()["my_scope"] = lua::Table{
		{"my_string", "some text"},
		{"my_data", 0.9234}
	};

	return 0;
}