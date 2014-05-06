#pragma once

#include "luapp11/fwd.hpp"

namespace luapp11 {
namespace internal {
// Invoking
template <typename T, class Enable = void>
struct caller {
  static result<T> call(lua_State* L, int nargs) {
    lua_call(L, nargs, 1);
    return popper<T>::get(L, -1);
  }
  static result<T> pcall(lua_State* L, int nargs) {
    auto err = lua_pcall(L, nargs, 1, 0);
    if (err) {
      return error(err, "Error calling lua method.", L);
    }
    return popper<T>::get(L, -1);
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
    return popper<std::tuple<TArgs...>>::get(L, (int)sizeof...(TArgs) * -1);
  }
  static result<std::tuple<TArgs...>> pcall(lua_State* L, int nargs) {
    auto err = lua_pcall(L, nargs, sizeof...(TArgs), 0);
    if (err) {
      return error(err, "Error calling lua method.", L);
    }
    return popper<std::tuple<TArgs...>>::get(L, (int)sizeof...(TArgs) * -1);
  }
};
}
}