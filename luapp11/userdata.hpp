#pragma once

#include <string>
#include "luapp11/var.hpp"

namespace luapp11 {

template <typename TDerived> class userdata {
 public:
  virtual ~userdata() = default;

 protected:
  userdata(const var& v) : type_hash_code_ { typeid(TDerived).hash_code() }
  { v.setup_metatable<TDerived>(&userdata::init_func); }

  template <typename T> void export_method(std::string& method_name) {}
 private:
  static void init_func(lua_State* L) {
      registrar<decltype((val(*)(val, val))&TDerived::operator+)>::reg(L, "__add");
      registrar<decltype((val(*)(val, val))&TDerived::operator-)>::reg(L, "__sub");
      registrar<decltype((val(*)(val, val))&TDerived::operator*)>::reg(L, "__mul");
      registrar<decltype((val(*)(val, val))&TDerived::operator/)>::reg(L, "__div");
      registrar<decltype((val(*)(val, val))&TDerived::operator%)>::reg(L, "__mod");
      registrar<decltype((val(*)(val, val)) & TDerived::pow)>::reg(L, "__pow");
      registrar<decltype(&TDerived::operator-)>::reg(L, "__unm");
      registrar<decltype((val(*)(val, val)) & TDerived::concat)>::reg(
          L, "__concat");
      registrar<decltype(&TDerived::length)>::reg(L, "__len");
      registrar<decltype((bool(*)(val, val))&TDerived::operator==)>::reg(L, "__eq");
      registrar<decltype((bool(*)(val, val))&TDerived::operator<)>::reg(L, "__lt");
      registrar<decltype((bool(*)(val, val))&TDerived::operator<=)>::reg(L, "__le");
      registrar<decltype((val(TDerived::*)(val))&TDerived::operator[])>::reg(L, "__index");
      registrar<decltype((val(TDerived::*)(val)) & TDerived::newindex)>::reg(
          L, "__newindex");
      registrar<decltype(&userdata::call)>::reg(L, "__call");
      registrar<decltype(&userdata::destroy)>::reg(L, "__gc");
  }

  int call(lua_State* L) {}

  int destroy(lua_State* L) {
    auto ptr = lua_touserdata(L, -1);
    auto p = (TDerived*)ptr;
    p->~TDerived();
  }

  template <typename T, class Enable = void> struct registrar {
    // static void reg(const var& v, std::string name) {}
  };

  template <typename T>
  struct registrar<T,
                   typename std::enable_if<
                       std::is_member_function_pointer<T>::value>::type> {
    // static void reg(const var & v, std::string name) {
    //   T func;
    //   v.register_meta_method(name, std::bind(func, t));
    // }
  };

  template <typename T>
  struct registrar<T,
                   typename std::enable_if<std::is_function<T>::value>::type> {
    void reg(lua_State* L, std::string name) {
      T func;
      lua_pushstring(L, name.c_str());
      val::pusher<T>::push(func);
      lua_settable(L, -3);
    }
  };

  size_t type_hash_code_;
};

}