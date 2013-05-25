#include "catch.hpp"
#include "lua/lua.hpp"

using namespace lua;

TEST_CASE("var_test/copy", "var copy test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(node);

  REQUIRE(node2.get<int>() == val);
}

TEST_CASE("var_test/move", "var move test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(std::move(node));

  REQUIRE(node2.get<int>() == val);
}

TEST_CASE("var_test/get_value", "get_value test") {
  int val = 10;
  auto node = root["test"] = val;
  auto val2 = node.get_value();

  REQUIRE(val2 == val);
}

TEST_CASE("var_test/get", "get test") {
  int val = 10;
  auto node = root["test"] = val;
  auto val2 = node.get<int>();

  REQUIRE(val2 == val);
  REQUIRE(node.get<std::string>() == "10");
}

TEST_CASE("var_test/is", "is test") {
  int val = 10;
  auto node = root["test"] = val;

  REQUIRE(node.is<int>());
  REQUIRE(node.is<bool>());
  REQUIRE(node.is<std::string>());
  REQUIRE(!node.is<std::function<void()>>());
}

TEST_CASE("var_test/as", "as test") {
  int val = 10;
  auto node = root["test"] = val;

  REQUIRE(node.as<int>(100) == 10);
  REQUIRE(node.as<bool>(false) == true);
  REQUIRE(node.as<std::string>("shoes") == "10");
  REQUIRE(node.as<std::tuple<int>>(std::make_tuple(13)) ==
          std::make_tuple(13));
}

TEST_CASE("var_test/assign", "assign test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2 = root["test2"] = node;

  REQUIRE(node.get<int>() == node2.get<int>());
}

TEST_CASE("var_test/equality", "equality tests") {
  auto node = root["test"]["foo"];
  REQUIRE(node == node);
  REQUIRE(node == root["test"]["foo"]);
  REQUIRE(root["test"][1] == root["test"][1]);
  REQUIRE(root["test"] != root["test"]["foo"]);
  REQUIRE(root["test"]["foo"] != root["test"][1]);
}

TEST_CASE("var_test/operator()", "operator() test") {
  auto func = root["func"] = chunk(
    R"PREFIX(
    function ()
      local v = i
    end
    )PREFIX");
  REQUIRE_NOTHROW(func());
}