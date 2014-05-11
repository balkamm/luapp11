#pragma once

#include "lua.hpp"
#include <utility>

namespace luapp11 {
namespace internal {
struct stack_guard {
  lua_State* state_;
  int initTop_;
  bool ret_;
  stack_guard(lua_State* state, bool ret = false)
      : state_{state}, initTop_{lua_gettop(state)}, ret_{ret} {}
  stack_guard(const stack_guard&) = delete;
  stack_guard(stack_guard&& other) {
    using std::swap;
    swap(state_, other.state_);
    swap(initTop_, other.initTop_);
    swap(ret_, other.ret_);
  }

  ~stack_guard() {
    auto toPop = lua_gettop(state_) - initTop_;
    if (ret_) {
      toPop--;
    }
    if (toPop > 0) {
      lua_pop(state_, toPop);
    }
  }
};
}
}