#pragma once

#include "lua.hpp"
#include <utility>

namespace luapp11 {

struct stack_guard {
  lua_State* state_;
  int initTop_;
  bool ret_;
  stack_guard(lua_State* state, bool ret = false) : state_ { state }
  , initTop_ { lua_gettop(state) }
  , ret_ { ret }
  {}
  stack_guard(const stack_guard&) = delete;

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