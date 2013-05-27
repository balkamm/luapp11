#include "catch.hpp"
#include "lua/lua.hpp"

using namespace lua;

TEST_CASE("val_test/create", "create test") {
	val v = {
		{"int", 10},
		{"bool",true},
		{"string","foo"},
		{"nil", val::nil()}
	};
	val::table_type values;
	CHECK_NOTHROW(values = v.get<val::table_type>());
	CHECK(values["int"] == 10);
	CHECK(values["bool"] == true);
	CHECK(values["string"] == "foo");
	CHECK(values["nil"] == val::nil());
}

TEST_CASE("val_test/get", "get test") {
	val v1(10);
	CHECK(v1.get<int>() == 10);
	CHECK(v1.get<float>() == 10.0f);
	CHECK(v1.get<bool>() == true);
	CHECK(v1.get<std::string>() == "10");
	CHECK_THROWS(v1.get<void*>());

	val v2("foo");
	CHECK(v2.get<std::string>() == "foo");
	CHECK(v2.get<bool>() == true);
	CHECK_THROWS(v2.get<int>());
	CHECK_THROWS(v2.get<void*>());

	val v3("10");
	CHECK(v3.get<std::string>() == "10");
	CHECK(v3.get<int>() == 10);
	CHECK(v3.get<bool>() == true);
	CHECK_THROWS(v3.get<void*>());

	val v4(true);
	CHECK(v4.get<bool>() == true);
	CHECK(v4.get<int>() == 1);
	CHECK_THROWS(v4.get<std::string>());
	CHECK_THROWS(v4.get<void*>());

	auto nil = val::nil();
	CHECK(nil.get<void*>() == (void*)nullptr);
	CHECK(nil.get<bool>() == false);
	CHECK(nil.get<int>() == 0);
	CHECK_THROWS(nil.get<std::string>());
}

TEST_CASE("val_test/equals", "equals test") {
	int ud;
	CHECK(val(10) == val(10));
	CHECK(val(10) == val(10.0f));
	CHECK(val(10) != val("10"));
	CHECK(val(10) != val(9));
	CHECK(val(10) != val(&ud));
	CHECK(val(10) != val::nil());

	CHECK(val("foo") == val("foo"));
	CHECK(val("foo") != val("10"));
	CHECK(val("foo") != val(10));
	CHECK(val("foo") != val(&ud));
	CHECK(val("foo") != val::nil());

	CHECK(val(&ud) == val(&ud));
	CHECK(val(&ud) != val(true));
	CHECK(val(&ud) != val("&ud"));
	CHECK(val(&ud) != val(10));
	CHECK(val(&ud) != val::nil());

	CHECK(val(true) == val(true));
	CHECK(val(true) != val(false));
	CHECK(val(true) != val("true"));
	CHECK(val(true) != val(10));
	CHECK(val(true) != val::nil());
}

TEST_CASE("val_test/assign", "assign test") {
	val v1(10);
	auto v2 = v1;
	CHECK(v1 == v2);
}