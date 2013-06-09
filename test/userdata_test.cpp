#include "catch.hpp"
#include "luapp11/lua.hpp"

using namespace luapp11;

class MyInt : public userdata<MyInt> {
 public:
  int get_int() const { return myInt_; }
 private:
  MyInt(int i) : userdata<MyInt>(), myInt_ { i }
  {}

  int myInt_;
  friend class var;
};

TEST_CASE("userdata_test/create", "userdata create test") {
  auto test = global["test"];
  ptr<MyInt> p = test.create<MyInt>(7);
  CHECK((*p).get_int() == 7);
}