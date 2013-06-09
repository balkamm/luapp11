#pragma once

#include <typeinfo>
#include <string>
#include "luapp11/internal/fwd.hpp"

namespace luapp11 {

class val;

template <typename TDerived> class userdata {
 public:
  virtual ~userdata() = default;

 protected:
  userdata() : type_hash_code_ { typeid(TDerived).hash_code() }
  {
    // v.setup_metatable<TDerived>(&userdata::init_func);
  }

  template <typename T> void export_method(std::string& method_name) {}
 private:
  static void init_func(lua_State* L) {
    register_add<TDerived>::reg(L);
      // registrar<decltype((val(*)(val, val))&TDerived::operator-)>::reg(L, "__sub");
      // registrar<decltype((val(*)(val, val))&TDerived::operator*)>::reg(L, "__mul");
      // registrar<decltype((val(*)(val, val))&TDerived::operator/)>::reg(L, "__div");
      // registrar<decltype((val(*)(val, val))&TDerived::operator%)>::reg(L, "__mod");
      // registrar<decltype((val(*)(val, val)) & TDerived::pow)>::reg(L, "__pow");
      // registrar<decltype(&TDerived::operator-)>::reg(L, "__unm");
      // registrar<decltype((val(*)(val, val)) & TDerived::concat)>::reg(
      //     L, "__concat");
      // registrar<decltype(&TDerived::length)>::reg(L, "__len");
      // registrar<decltype((bool(*)(val, val))&TDerived::operator==)>::reg(L, "__eq");
      // registrar<decltype((bool(*)(val, val))&TDerived::operator<)>::reg(L, "__lt");
      // registrar<decltype((bool(*)(val, val))&TDerived::operator<=)>::reg(L, "__le");
      // registrar<decltype((val(TDerived::*)(val))&TDerived::operator[])>::reg(L, "__index");
      // registrar<decltype((val(TDerived::*)(val)) & TDerived::newindex)>::reg(
      //     L, "__newindex");
      // // registrar::reg(L, "__call", dispatch());
      // registrar<decltype(&userdata::destroy)>::reg(L, "__gc");
  }

  static int call(lua_State* L) {}

  static int destroy(lua_State* L) {
    auto ptr = lua_touserdata(L, -1);
    auto p = (TDerived*)ptr;
    p->~TDerived();
  }

  static bool is(void* ptr) {
    return ((userdata<TDerived>*)ptr)->type_hash_code_ ==
           typeid(TDerived).hash_code();
  }

  static TDerived* cast(void* ptr) {
    if (!is(ptr)) {
      throw luapp11::exception("Trying to cast userdata to wrong type.");
    }
    return (TDerived*)ptr;
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
      detail::push_func(func);
      lua_settable(L, -3);
    }
  };

  template <typename T, class Enable = void> struct register_add {
    static void reg(lua_State* L) {}
  };

  template <typename T> struct register_add<T,typename std::enable_if<std::is_function<decltype(&T::operator+)>::value>::type> {
    static void reg(lua_State* L) {
      lua_pushstring(L, "__add");
      detail::push_func(&T::operator+);
      lua_settable(L, -3);
    }
  };

  size_t type_hash_code_;
  friend class var;
  friend class val;
};

}