#pragma once

#include "luapp11/fwd.hpp"

namespace luapp11 {
namespace internal {

// Is checking
template <typename T, class Enable = void>
struct typed_is {
  static inline bool is(lua_State* L, int idx = -1) { return false; }
};

template <typename T>
struct typed_is<T,
                typename std::enable_if<std::is_arithmetic<T>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) &&
           (lua_isboolean(L, idx) || lua_isnumber(L, idx));
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) && lua_isstring(L, idx);
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) && lua_isstring(L, idx);
  }
};

template <typename T>
struct typed_is<T, typename std::enable_if<std::is_function<T>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) && lua_isfunction(L, idx);
  }
};

template <typename T>
struct typed_is<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) && lua_islightuserdata(L, idx);
  }
};

template <typename T>
struct typed_is<
    T, typename std::enable_if<std::is_base_of<userdata<T>, T>::value>::type> {
  static inline bool is(lua_State* L, int idx = -1) {
    return !lua_isnoneornil(L, idx) && lua_isuserdata(L, idx) &&
           userdata<T>::is(lua_touserdata(L, idx));
  }
};
}
}