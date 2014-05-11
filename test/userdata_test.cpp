#include "catch.hpp"
#include "luapp11/lua.hpp"

using namespace luapp11;

class MyInt : public userdata<MyInt> {
 public:
  int get_int() const { return myInt_; }

 private:
  MyInt(int i) : userdata<MyInt>(), myInt_{i} {}

  friend MyInt operator+(const MyInt& a, const MyInt& b) {
    std::cout << "Special Add..." << std::endl;
    return a.myInt_ + b.myInt_;
  }
  int myInt_;
  friend class var;
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