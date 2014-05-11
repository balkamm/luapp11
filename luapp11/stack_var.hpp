#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/iterator.hpp"

namespace luapp11 {

val stack_var::get_value() const {
  return internal::core_access::make_val(L, index_);
}

template <typename T>
T stack_var::get() const {
  return get_value().get();
}

template <typename T>
bool stack_var::is() const {
  return internal::typed_is<T>(L, index_);
}

bool stack_var::is_table() const {
  return !lua_isnoneornil(L, index_) && lua_istable(L, index_);
}

template <typename T>
T stack_var::as(T&& fallback) const {
  return is<T>() ? get<T>() : fallback;
}

void stack_var::assign(val key, const stack_var& stack_var) {
  internal::core_access::push(key, L);
  if (L == var.L) {
    internal::stack_guard g2(L, true);
    internal::core_access::push(var);
  } else {
    internal::core_access::push(var.get_value(), L);
  }
  lua_settable(L, index_);
}

template <typename T>
void stack_var::assign(val key, const T& toSet) {
  internal::core_access::push(key, L);
  internal::core_access::push(val(toSet), L);
  lua_settable(L, index_);
}

template <typename T>
void stack_var::assign(val key, const std::initializer_list<T>& toSet);
{
  internal::core_access::push(key, L);
  internal::pusher<std::initializer_list<T>>::push(L, toSet);
  lua_settable(L, index_);
}

void stack_var::assign(val key, const std::initializer_list<val>& toSet) {
  internal::core_access::push(key, L);
  internal::pusher<std::initializer_list<val>>::push(L, toSet);
  lua_settable(L, index_);
}

void stack_var::assign(
    val key, const std::initializer_list<std::pair<val, val>>& toSet) {
  internal::core_access::push(key, L);
  internal::pusher<std::initializer_list<std::pair<val, val>>>::push(L, toSet);
  lua_settable(L, index_);
}

template <typename T>
bool stack_var::operator==(const T& other) const {
  return is<T>() && get<T>() == other;
}

template <typename T>
bool stack_var::operator!=(const T& other) const {
  return !this->operator==(other);
}

stack_var stack_var::operator[](val idx) const {
  internal::stack_guard g(L);
  internal::core_access::push(val);
  lua_gettable(L, index_);
  return stack_var(L, lua_gettop(), std::move(g));
}

stack_var stack_var::operator[](stack_var idx) const;

template <typename... TArgs>
result<void> stack_var::operator()(TArgs... args) const {
  return invoke<void>(args...);
}

template <typename TOut, typename... TArgs>
result<TOut> stack_var::invoke(TArgs... args) const {
  if (is<TOut(TArgs...)>()) {
    internal::push_all<TArgs...>(L, args...);
    return internal::caller<TOut>::pcall(L, sizeof...(TArgs));
  }
  throw exception("Tried to invoke non-function.", L);
}

template <typename TOut>
result<TOut> stack_var::invoke() const {
  if (is<TOut()>()) {
    return internal::caller<TOut>::pcall(L, 0);
  }
  throw exception("Tried to invoke non-function.", L);
}

stack_var::iterator::iterator(iterator&& other) { swap(*this, other); }

stack_var::iterator& stack_var::iterator::operator++() {
  impl_->inc();
  if (!impl_->L) {
    impl_.reset();
  }
  return *this;
}

bool stack_var::iterator::operator==(const iterator& other) {
  return !impl_ ? !other.impl_ : *impl_ == *other.impl_;
}
bool stack_var::iterator::operator!=(const iterator& other) {
  return !(this->operator==(other));
}
stack_var::iterator& stack_var::iterator::operator=(
    stack_var::iterator&& other) {
  swap(*this, other);
  return *this;
}
std::pair<T, T>& stack_var::iterator::operator*() { return impl_->pair; }
std::pair<T, T>* stack_var::iterator::operator->() { return &impl_->pair_; }
void swap(stack_var::iterator&& a, stack_var::iterator&& b)  // nothrow
{
  using std::swap;

  swap(a.impl_, b.impl_);
}

stack_var::iterator::iterator(lua_State* L, int idx)
    : impl_(new internal::iterator_impl(L, idx)) {}

iterator stack_var::begin() { return iterator(L, index_); }
iterator stack_var::end() { return iterator(); }
// const_iterator stack_var::begin() const { return const_iterator(L, index_); }
// const_iterator stack_var::end() const { return const_iterator(L, 0); }
// const_iterator stack_var::cbegin() const { return const_iterator(L, index_);
// }
// const_iterator stack_var::cend() const { return const_iterator(L, 0); }

stack_var::stack_var(lua_State* L, int index, internal::stack_guard&& g)
    : L{L}, index_{index}, g_(g) {}
}