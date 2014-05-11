#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/stack_guard.hpp"
#include "luapp11/var.hpp"

namespace luapp11 {
namespace internal {
struct iterator_impl {
 public:
  iterator_impl(lua_State* L, int idx)
      : L(L),
        idx(idx),
        pair(internal::core_access::make_stack_var(L, lua_gettop(L) + 1,
                                                   internal::stack_guard(L)),
             internal::core_access::make_stack_var(L, lua_gettop(L) + 2,
                                                   internal::stack_guard(L))) {
    lua_pushnil(L);
    inc();
  }

  bool operator==(const iterator_impl& other) {
    return std::tie(L, idx, pair) == std::tie(other.L, other.idx, other.pair);
  }

  void inc() {
    if (!lua_next(L, idx)) {
      L = nullptr;
    }
  }

  lua_State* L;
  int idx;
  std::pair<stack_var, stack_var> pair;
};
}
}
