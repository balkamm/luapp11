#pragma once

namespace luapp11 {
namespace internal {

// Is checking
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
}
}