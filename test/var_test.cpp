#include "catch.hpp"
#include "luapp11/lua.hpp"

using namespace luapp11;

TEST_CASE("var_test/copy", "var copy test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(node);

  CHECK(node2.get<int>() == val);

  auto node3(root["dne"]);
  CHECK(node3 == root["dne"]);
}

TEST_CASE("var_test/move", "var move test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2(std::move(node));

  CHECK(node2.get<int>() == val);

  auto dne = root["dne"];
  auto node3(std::move(dne));

  CHECK(node3 == root["dne"]);
}

TEST_CASE("var_test/get_value", "get_value test") {
  int val = 10;
  auto node = root["test"] = val;
  auto val2 = node.get_value();

  CHECK(val2 == val);
  CHECK(root["dne"].get_value() == val::nil());
}

TEST_CASE("var_test/get", "get test") {
  int val = 10;
  auto node = root["test"] = val;
  CHECK(node.get<int>() == val);
  CHECK(node.get<std::string>() == "10");
  CHECK(node.get<bool>() == true);

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  CHECK(node2.get<std::string>() == val2);
  CHECK(node2.get<bool>() == true);
  CHECK_THROWS(node2.get<int>());

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  CHECK(node3.get<bool>() == val3);
  CHECK(node3.get<int>() == 1);
  CHECK_THROWS(node3.get<std::string>());

  CHECK(root["dne"].get<int>() == 0);
}

TEST_CASE("var_test/is", "is test") {
  int val = 10;
  auto node = root["test"] = val;
  CHECK(node.is<int>());
  CHECK(node.is<bool>());
  CHECK(node.is<std::string>());
  CHECK(!node.is<std::function<void()>>());

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  CHECK(!node2.is<int>());
  CHECK(!node2.is<bool>());
  CHECK(node2.is<std::string>());
  CHECK(!node2.is<std::function<void()>>());

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  CHECK(node3.is<int>());
  CHECK(node3.is<bool>());
  CHECK(!node3.is<std::string>());
  CHECK(!node3.is<std::function<void()>>());

  CHECK(!root["dne"].is<int>());
  CHECK(!root["dne"].is<bool>());
  CHECK(!root["dne"].is<std::string>());
  CHECK(!root["dne"].is<std::function<void()>>());
}

TEST_CASE("var_test/as", "as test") {
  int val = 10;
  auto node = root["test"] = val;
  CHECK(node.as<int>(100) == 10);
  CHECK(node.as<bool>(false) == true);
  CHECK(node.as<std::string>("shoes") == "10");

  std::string val2 = "foo";
  auto node2 = root["test2"] = val2;
  CHECK(node2.as<int>(100) == 100);
  CHECK(node2.as<bool>(false) == false);
  CHECK(node2.as<std::string>("shoes") == "foo");

  bool val3 = true;
  auto node3 = root["test3"] = val3;
  CHECK(node3.as<int>(100) == 1);
  CHECK(node3.as<bool>(false) == true);
  CHECK(node3.as<std::string>("shoes") == "shoes");

  CHECK(root["dne"].as<int>(100) == 100);
  CHECK(root["dne"].as<bool>(false) == false);
  CHECK(root["dne"].as<std::string>("shoes") == "shoes");
}

TEST_CASE("var_test/assign", "assign test") {
  int val = 10;
  auto node = root["test"] = val;
  auto node2 = root["test2"] = node;

  CHECK(node.get<int>() == node2.get<int>());
  CHECK(node != node2);
}

TEST_CASE("var_test/equality", "equality tests") {
  auto node = root["test"]["foo"];
  CHECK(node == node);
  CHECK(node == root["test"]["foo"]);
  CHECK(root["test"][1] == root["test"][1]);
  CHECK(root["test"] != root["test"]["foo"]);
  CHECK(root["test"]["foo"] != root["test"][1]);
}

TEST_CASE("var_test/do_chunk", "do_chunk test") {
  auto node = root["test"];
  auto err = node.do_chunk("return 15");
  CHECK(!(bool)err);
  CHECK(node.get<int>() == 15);

  auto node2 = root["test2"];
  auto err2 = node2.do_chunk("Invalid LUA;;");
  CHECK((bool)err2);
}

TEST_CASE("var_test/operator()", "operator() test") {
  auto func = root["func"];
  func.do_chunk(
      R"PREFIX(
    return function ()
      local v = 5
    end
    )PREFIX");
  CHECK_NOTHROW(func());
  CHECK_THROWS(root["foo"]);
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
  CHECK(result.success());
  CHECK(result.value() == 12);
  CHECK_THROWS(root["foo"].invoke<int>());
}