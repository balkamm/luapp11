#pragma once

#include <type_traits>
#include "luapp11/var.hpp"

namespace luapp11 {
namespace internal {

/**
 * typed_is::is pushes a value onto the stack checks to see if it's convertible
 * to
 * the given type.  It leaves the value on the stack.
 */
template <typename T, class Enable = void>
struct typed_is {
  static inline bool is(lua_State* L) { return false; }
};

template <typename T>
struct typed_is<T,
                typename std::enable_if<std::is_arithmetic<T>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) &&
           (lua_isboolean(L, -1) || lua_isnumber(L, -1));
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) && lua_isstring(L, -1);
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) && lua_isstring(L, -1);
  }
};

template <typename T>
struct typed_is<T, typename std::enable_if<std::is_function<T>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) && lua_isfunction(L, -1);
  }
};

template <typename T>
struct typed_is<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) && lua_islightuserdata(L, -1);
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_base_of<userdata<T>, T>::value>::type> {
  static inline bool is(lua_State* L) {
    return !lua_isnoneornil(L, -1) && lua_isuserdata(L, -1) &&
           userdata<T>::is(lua_touserdata(L, -1));
  }
};

/**
 * caller::call calls the function on the stack, and returns/pops the
 * return values.  Returns a tuple if multiple values are returned.
 */
template <typename T, class Enable = void>
struct caller {
  static result<T> call(lua_State* L, int nargs) {
    lua_call(L, nargs, 1);
    return val::popper<T>::get(L, -1);
  }
  static result<T> pcall(lua_State* L, int nargs) {
    auto err = lua_pcall(L, nargs, 1, 0);
    if (err) {
      return error(err, "Error calling lua method.", L);
    }
    return val::popper<T>::get(L, -1);
  }
};

template <typename T>
struct caller<T, typename std::enable_if<std::is_void<T>::value>::type> {
  static result<T> call(lua_State* L, int nargs) {
    lua_call(L, nargs, 1);
    return result<T>();
  }
  static result<T> pcall(lua_State* L, int nargs) {
    auto err = lua_pcall(L, nargs, 1, 0);
    if (err) {
      return error(err, "Error calling lua method", L);
    }
    return result<T>();
  }
};

template <typename... TArgs>
struct caller<std::tuple<TArgs...>, std::enable_if<true>::type> {
  static result<std::tuple<TArgs...>> call(lua_State* L, int nargs) {
    lua_call(L, nargs, sizeof...(TArgs));
    return val::popper<std::tuple<TArgs...>>::get(L,
                                                  (int)sizeof...(TArgs) * -1);
  }
  static result<std::tuple<TArgs...>> pcall(lua_State* L, int nargs) {
    auto err = lua_pcall(L, nargs, sizeof...(TArgs), 0);
    if (err) {
      return error(err, "Error calling lua method.", L);
    }
    return val::popper<std::tuple<TArgs...>>::get(L,
                                                  (int)sizeof...(TArgs) * -1);
  }
};
}
}