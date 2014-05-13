#pragma once

#include <typeinfo>
#include <string>
#include "luapp11/val.hpp"
#include "luapp11/internal/stack.hpp"

namespace luapp11 {

template <typename TDerived>
class userdata {
 public:
  virtual ~userdata() = default;

  static void init_func(lua_State* L) { register_add<TDerived>::reg(L); }

 protected:
  userdata() : type_hash_code_{typeid(TDerived).hash_code()} {
    // v.setup_metatable<TDerived>(&userdata::init_func);
  }

  template <typename T>
  void export_method(std::string& method_name) {}

 private:
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

  // template <typename T, class Enable = void>
  // struct registrar {
  //   // static void reg(const var& v, std::string name) {}
  // };

  // template <typename T>
  // struct registrar<T, typename std::enable_if<
  //                         std::is_member_function_pointer<T>::value>::type> {
  //   // static void reg(const var & v, std::string name) {
  //   //   T func;
  //   //   v.register_meta_method(name, std::bind(func, t));
  //   // }
  // };

  // template <typename T>
  // struct registrar<T,
  //                  typename std::enable_if<std::is_function<T>::value>::type>
  //                  {
  //   void reg(lua_State* L, std::string name) {
  //     T func;
  //     lua_pushstring(L, name.c_str());
  //     internal::push(func);
  //     lua_settable(L, -3);
  //   }
  // };

  template <typename T, class Enable = void>
  struct register_add {
    static void reg(lua_State* L) {}
  };

  template <typename T>
  struct register_add<T,
                      typename std::enable_if<std::is_member_function_pointer<
                          decltype(&T::operator+)>::value>::type> {
    static void reg(lua_State* L) {
      lua_pushstring(L, "__add");
      internal::push(L, &T::operator+);
      lua_settable(L, -3);
    }
  };

  size_t type_hash_code_;
  friend class var;
  friend class val;
};
}