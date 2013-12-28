#pragma once

namespace luapp11 {
class var;
namespace internal {
template <typename T>
class iterator
    : public std::iterator<std::forward_iterator_tag, std::pair<val, T>> {
 public:
  iterator(const var& v);
  iterator();
  iterator(const iterator&) = delete;
  iterator(iterator&& other);
  iterator& operator++();

  bool operator==(const iterator& other);
  bool operator!=(const iterator& other);
  iterator& operator=(iterator other);

  std::pair<val, T> operator*();

  friend void swap(iterator&& a, iterator&& b);

 private:
  void inc();

  const var* v_;
  lua_State* L;
  int idx_;
  std::unique_ptr<stack_guard> g_;
  friend class var;
};
}

using const_var_iterator = internal::iterator<const var>;
using var_iterator = internal::iterator<var>;
}
