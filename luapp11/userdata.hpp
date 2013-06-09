#pragma once

#include <string>
#include "luapp11/var.hpp"

namespace luapp11 {

template <typename TDerived> class userdata {
 public:
  virtual ~userdata() = default;

 protected:
  userdata(const var& v) {
    registrar<decltype((val(*)(val, val))&TDerived::operator+)>::reg(v, "__add");
    registrar<decltype((val(*)(val, val))&TDerived::operator-)>::reg(v, "__sub");
    registrar<decltype((val(*)(val, val))&TDerived::operator*)>::reg(v, "__mul");
    registrar<decltype((val(*)(val, val))&TDerived::operator/)>::reg(v, "__div");
    registrar<decltype((val(*)(val, val))&TDerived::operator%)>::reg(v, "__mod");
    registrar<decltype((val(*)(val, val))&TDerived::pow)>::reg(v, "__pow");
    registrar<decltype(&TDerived::operator-)>::reg(v, "__unm");
    registrar<decltype((val(*)(val, val))&TDerived::concat)>::reg(v, "__concat");
    registrar<decltype(&TDerived::length)>::reg(v, "__len");
    registrar<decltype((bool(*)(val, val))&TDerived::operator==)>::reg(v, "__eq");
    registrar<decltype((bool(*)(val, val))&TDerived::operator<)>::reg(v, "__lt");
    registrar<decltype((bool(*)(val, val))&TDerived::operator<=)>::reg(v, "__le");
    registrar<decltype((val(TDerived::*)(val))&TDerived::operator[])>::reg(v, "__index");
    registrar<decltype((val(TDerived::*)(val))&TDerived::newindex)>::reg(v, "__newindex");
    registrar<decltype(&userdata::call)>::reg(v, "__call");
    registrar<decltype(&userdata::destroy)>::reg(v, "__gc");
  }

  template<typename T>
  void export_method(std::string& method_name) {

  }
private:
  int call(lua_State* L) {

  }

  int destroy(lua_State* L) {
    auto ptr = lua_touserdata(L, -1);
    auto p = (TDerived*)ptr;
    p->~TDerived();
  }

  template <typename T, class Enable = void> struct registrar {
    static void reg(const var& v, std::string name) {}
  };

  template <typename T>
  struct registrar<T, typename std::enable_if<std::is_member_function_pointer<T>::value>::type> {
    // static void reg(const var & v, std::string name) {
    //   T func;
    //   v.register_meta_method(name, std::bind(func, t));
    // }
  };

  template <typename T>
  struct registrar<T, typename std::enable_if<std::is_function<T>::value>::type> {
    void reg(const var & v, std::string name) {
      T func;
      v.register_meta_method(name, func);
    }
  };
};

}