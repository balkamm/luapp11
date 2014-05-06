#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/stack_guard.hpp"
#include "luapp11/var.hpp"

namespace luapp11 {
namespace internal {

template <typename T>
iterator<T>::iterator(const var& v)
    : v_(&v), L(core_access::state(v)), g_(new stack_guard(L)) {
  core_access::push(v);
  idx_ = lua_gettop(L);
  lua_pushnil(L);
  inc();
}
template <typename T>
iterator<T>::iterator(iterator&& other) {
  swap(*this, other);
}
template <typename T>
iterator<T>& iterator<T>::operator++() {
  inc();
  return *this;
}

template <typename T>
bool iterator<T>::operator==(const iterator& other) {
  return std::tie(L, idx_) == std::tie(other.L, other.idx_);
}
template <typename T>
bool iterator<T>::operator!=(const iterator& other) {
  return std::tie(L, idx_) == std::tie(other.L, other.idx_);
}
template <typename T>
iterator<T>& iterator<T>::operator=(iterator<T> other) {
  swap(*this, other);
  return *this;
}
template <typename T>
std::pair<val, T>& iterator<T>::operator*() {
  return pair_;
}
template <typename T>
std::pair<val, T>* iterator<T>::operator->() {
  return &pair_;
}
template <typename T>
void swap(iterator<T>&& a, iterator<T>&& b)  // nothrow
{
  using std::swap;

  swap(a.v_, b.v_);
  swap(a.L, b.L);
  swap(a.idx_, b.idx_);
  swap(a.g_, b.g_);
  swap(a.pair_, b.pair_);
}

template <typename T>
iterator<T>::iterator(lua_State* L, int idx)
    : L(L), idx_(idx), v_(nullptr) {}

template <typename T>
void iterator<T>::inc() {
  if (!lua_next(L, idx_)) {
    idx_ = 0;
  }
  auto val = core_access::make_val(L, -2);
  pair_ = std::make_pair(val, (*v_)[pair_.first]);
}
}
}
