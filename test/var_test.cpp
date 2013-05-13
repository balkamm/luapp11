#include "catch.hpp"
#include "lua/lua.hpp"

TEST_CASE("var_test/copy", "var copy test") {
	int val = 10;
	auto node = lua::root["test"] = val;
	auto node2(node);

	REQUIRE(node2 == node);
	REQUIRE(node2.get<int>() == val);
}

TEST_CASE("var_test/move", "var move test") {
	int val = 10;
	auto node = lua::root["test"] = val;
	auto node2(std::move(node));

	REQUIRE(node2 == lua::root["test"]);
	REQUIRE(node2.get<int>() == val);	
}