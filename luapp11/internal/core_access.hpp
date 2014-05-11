#pragma once

#include "luapp11/fwd.hpp"
#include <utility>

#define ACCESS_MEMBER(type, mtype, name) \
  static mtype get_##type##_##name(type t) { return t.name; }

namespace luapp11 {
namespace internal {
class core_access {
 public:
  static val make_val(lua_State* L, val::type t, int idx) {
    return val(L, t, idx);
  }
  static val make_val(lua_State* L, val::type t) { return val(L, t); }
  static val make_val(lua_State* L, int idx) { return val(L, idx); }

  static stack_var make_stack_var(lua_State* L, int idx, stack_guard&& g) {
    return stack_var(L, idx, std::forward<stack_guard>(g));
  }
  static stack_var make_stack_var() {
    return stack_var(nullptr, 0, stack_guard(nullptr));
  }

  static val make_val(lua_State* L) { return val(L); }
  static var make_var(lua_State* L, int virtual_index, val key) {
    return var(L, virtual_index, key);
  }
  static var make_var(var v, val key) { return var(v, key); }

  static void push(const val& v, lua_State* L) { v.push(L); }
  static void push(const var& v) { v.push(); }

  static lua_State* state(const var& v) { return v.L; }

  ACCESS_MEMBER(val, void*, ptr);
  ACCESS_MEMBER(val, bool, boolean);
  ACCESS_MEMBER(val, lua_Number, num);
  ACCESS_MEMBER(val, const char*, str);
  ACCESS_MEMBER(val, lua_State*, thread);
  ACCESS_MEMBER(val, std::shared_ptr<val::table_type>, table);
};
}
}

#undef ACCESS_MEMBER