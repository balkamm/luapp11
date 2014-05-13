#include "catch.hpp"
#include "luapp11/lua.hpp"

using namespace luapp11;

class MyInt : public userdata<MyInt> {
 public:
  MyInt(int i) : userdata<MyInt>(), myInt_{i} {}
  MyInt(const MyInt& other) : myInt_{other.myInt_} {}
  MyInt(MyInt&& other) : myInt_{std::move(other.myInt_)} {}
  int get_int() const { return myInt_; }

  int operator+(const MyInt& b) { return myInt_ + b.myInt_; }

 private:
  int myInt_;
};

TEST_CASE("userdata_test/create", "userdata create test") {
  auto test = global["test"];
  ptr<MyInt> p = test.create<MyInt>(7);
  CHECK(p->get_int() == 7);
  CHECK(test == p);
}

TEST_CASE("userdata_test/add", "userdata add op test") {
  ptr<MyInt> p1 = global["p1"].create<MyInt>(7);
  ptr<MyInt> p2 = global["p2"].create<MyInt>(5);
  global["p3"].do_chunk("return p1 + p2");
  CHECK(global["p3"].get<int>() == 12);
}