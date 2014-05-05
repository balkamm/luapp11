#pragma once

namespace luapp11 {
namespace internal {
template <typename T, class Enable = void>
struct do_create {};

template <typename T>
struct do_create<
    T, typename std::enable_if<std::is_base_of<userdata<T>, T>::value>::type> {
  template <typename... TArgs>
  static void create(const var& v, TArgs... args) {
    auto ptr = lua_newuserdata(v.L, sizeof(T));
    new (ptr) T(args...);
    v.setup_metatable<T>();
  }
};
}
}