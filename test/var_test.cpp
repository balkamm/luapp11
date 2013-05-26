#include "catch.hpp"
#include "lua/lua.hpp"

using namespace lua;

TEST_CASE("var_test/copy", "var copy test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(node);

  REQUIRE(node2.get<int>() == val);

  auto node3(root["dne"]);
  REQUIRE(node3 == root["dne"]);
}

TEST_CASE("var_test/move", "var move test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(std::move(node));

  REQUIRE(node2.get<int>() == val);

  auto dne = root["dne"];
  auto node3(std::move(dne));

  REQUIRE(node3 == root["dne"]);
}

TEST_CASE("var_test/get_value", "get_value test") {
  int val = 10;
  auto node = root["test"] = val;
  auto val2 = node.get_value();

  REQUIRE(val2 == val);
  REQUIRE(root["dne"].get_value() == val::nil());
}

TEST_CASE("var_test/get", "get test") {
  int val = 10;
  auto node = root["test"] = val;
  REQUIRE(node.get<int>() == val);
  REQUIRE(node.get<std::string>() == "10");
  REQUIRE(node.get<bool>() == true);

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  REQUIRE(node2.get<std::string>() == val2);
  REQUIRE_THROWS(node2.get<int>());
  REQUIRE_THROWS(node2.get<bool>());

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  REQUIRE(node3.get<bool>() == val3);
  REQUIRE(node3.get<int>() == 1);
  REQUIRE_THROWS(node3.get<std::string>());

  REQUIRE(root["dne"].get<int>() == 0);
}

TEST_CASE("var_test/is", "is test") {
  int val = 10;
  auto node = root["test"] = val;
  REQUIRE(node.is<int>());
  REQUIRE(node.is<bool>());
  REQUIRE(node.is<std::string>());
  REQUIRE(!node.is<std::function<void()>>());

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  REQUIRE(!node2.is<int>());
  REQUIRE(!node2.is<bool>());
  REQUIRE(node2.is<std::string>());
  REQUIRE(!node2.is<std::function<void()>>());

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  REQUIRE(node3.is<int>());
  REQUIRE(node3.is<bool>());
  REQUIRE(!node3.is<std::string>());
  REQUIRE(!node3.is<std::function<void()>>());

  REQUIRE(!root["dne"].is<int>());
  REQUIRE(!root["dne"].is<bool>());
  REQUIRE(!root["dne"].is<std::string>());
  REQUIRE(!root["dne"].is<std::function<void()>>());
}

TEST_CASE("var_test/as", "as test") {
  int val = 10;
  auto node = root["test"] = val;
  REQUIRE(node.as<int>(100) == 10);
  REQUIRE(node.as<bool>(false) == true);
  REQUIRE(node.as<std::string>("shoes") == "10");

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  REQUIRE(node2.as<int>(100) == 100);
  REQUIRE(node2.as<bool>(false) == false);
  REQUIRE(node2.as<std::string>("shoes") == "foo");

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  REQUIRE(node3.as<int>(100) == 1);
  REQUIRE(node3.as<bool>(false) == true);
  REQUIRE(node3.as<std::string>("shoes") == "shoes");

  REQUIRE(root["dne"].as<int>(100) == 100);
  REQUIRE(root["dne"].as<bool>(false) == false);
  REQUIRE(root["dne"].as<std::string>("shoes") == "shoes");
}

TEST_CASE("var_test/assign", "assign test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2 = root["test2"] = node;

  REQUIRE(node.get<int>() == node2.get<int>());
  REQUIRE(node != node2);
}

TEST_CASE("var_test/equality", "equality tests") {
  auto node = root["test"]["foo"];
  REQUIRE(node == node);
  REQUIRE(node == root["test"]["foo"]);
  REQUIRE(root["test"][1] == root["test"][1]);
  REQUIRE(root["test"] != root["test"]["foo"]);
  REQUIRE(root["test"]["foo"] != root["test"][1]);
}

TEST_CASE("var_test/do_chunk", "do_chunk test") {
  auto node = root["test"];
  auto err = node.do_chunk("return 15");
  REQUIRE(!(bool)err);
  REQUIRE(node.get<int>() == 15);

  auto node2 = root["test2"];
  auto err2 = node2.do_chunk("Invalid LUA;;");
  REQUIRE((bool)err2);
}

TEST_CASE("var_test/operator()", "operator() test") {
  auto func = root["func"];
  func.do_chunk(
      R"PREFIX(
    return function ()
      local v = 5
    end
    )PREFIX");
  REQUIRE_NOTHROW(func());
}

TEST_CASE("var_test/invoke", "invoke test") {
  auto func = root["func"];
  func.do_chunk(
      R"PREFIX(
    return function (i)
      return i + 5
    end
    )PREFIX");
  auto result = func.invoke<int>(7);
  REQUIRE(result.success());
  REQUIRE(result.value() == 12);
}