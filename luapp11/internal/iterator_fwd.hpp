#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/stack_guard.hpp"

namespace luapp11 {
class var;
namespace internal {
template <typename T>
class iterator
    : public std::iterator<std::forward_iterator_tag, std::pair<val, T>> {
 public:
  iterator(const var& v);
  iterator(const iterator&) = delete;
  iterator(iterator&& other);
  iterator& operator++();
  bool operator==(const iterator& other);
  bool operator!=(const iterator& other);
  iterator& operator=(iterator other);
  std::pair<val, T>& operator*();
  std::pair<val, T>* operator->();
  friend void swap(iterator&& a, iterator&& b);

 private:
  iterator(lua_State* L, int idx);
  void inc();

  const var* v_;
  lua_State* L;
  int idx_;
  std::unique_ptr<stack_guard> g_;
  std::pair<val, T> pair_;
  friend class luapp11::var;
};
}
}
