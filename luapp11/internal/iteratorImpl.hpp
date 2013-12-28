#pragma once

namespace luapp11 {
namespace internal {

template <typename T>
iterator<T>::iterator(const var& v)
    : v_(&v), L(v.L), g_(new stack_guard(L)) {
  v.push();
  idx_ = lua_gettop(L);
  lua_pushnil(L);
  inc();
}

template <typename T>
iterator<T>::iterator()
    : L(nullptr), idx_(0), v_(nullptr) {}

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
iterator<T>& iterator<T>::operator=(iterator other) {
  swap(*this, other);
  return *this;
}

template <typename T>
std::pair<val, T> iterator<T>::operator*() {
  val v(L);
  return std::make_pair(v, (*v_)[v]);
}

template <typename T>
void swap(iterator<T>&& a, iterator<T>&& b)  // nothrow
{
  using std::swap;

  swap(a.v_, b.v_);
  swap(a.L, b.L);
  swap(a.idx_, b.idx_);
  swap(a.g_, b.g_);
}

template <typename T>
void iterator<T>::inc() {
  if (!lua_next(L, idx_)) {
    L = 0;
    idx_ = 0;
  }
}
}
}
