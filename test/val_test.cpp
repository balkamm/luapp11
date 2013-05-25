#include "catch.hpp"
#include "lua/lua.hpp"

using namespace lua;

TEST_CASE("val_test/equals", "equals test") {
  REQUIRE(val(10) == val(10));
  REQUIRE(val("foo") == val("foo"));
  REQUIRE(val(true) == val(true));
  REQUIRE(val() == val());
}