#pragma once
#include "luapp11/var.hpp"
#include "luapp11/internal/utility.hpp"
#include "luapp11/internal/traits.hpp"

namespace luapp11 {
val var::get_value() const {
  stack_guard g(L);
  push();
  return val(L);
}

template <typename T>
T var::get() const {
  return get_value().get<T>();
}

template <typename T>
bool var::is() const {
  stack_guard g(L);
  return dirty_is<T>();
}

bool var::is_table() const {
  stack_guard g(L);
  return !lua_isnoneornil(L, -1) && lua_istable(L, -1);
}

template <typename T>
T var::as(T&& fallback) const {
  stack_guard g(L);
  return dirty_is<T>() ? val(L).get<T>() : fallback;
}

var& var::operator=(const var& var) {
  stack_guard g(L);
  push_parent_key();
  if (L == var.L) {
    stack_guard g2(L, true);
    var.push();
  } else {
    var.get_value().push(L);
  }
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return *this;
}

template <typename T>
var& var::operator=(const T& toSet) {
  stack_guard g(L);
  push_parent_key();
  val::pusher<typename detail::convert_functor_to_std_function<T>::type>::push(
      L, toSet);
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return *this;
}

template <typename T>
var& var::operator=(const std::initializer_list<T>& toSet) {
  stack_guard g(L);
  push_parent_key();
  val::pusher<std::initializer_list<T>>::push(L, toSet);
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return *this;
}

var& var::operator=(const std::initializer_list<val>& toSet) {
  stack_guard g(L);
  push_parent_key();
  val::pusher<std::initializer_list<val>>::push(L, toSet);
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return *this;
}

var& var::operator=(const std::initializer_list<std::pair<val, val>>& toSet) {
  stack_guard g(L);
  push_parent_key();
  val::pusher<std::initializer_list<std::pair<val, val>>>::push(L, toSet);
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return *this;
}

bool var::operator==(const var& other) const {
  return L == other.L && virtual_index_ == other.virtual_index_ &&
         lineage_.size() == other.lineage_.size() &&
         std::mismatch(lineage_.begin(), lineage_.end(), other.lineage_.begin())
                 .first == lineage_.end();
}

template <typename T>
bool var::operator==(const T& other) const {
  return get<T>() == other;
}

bool var::operator!=(const var& other) const { return !(operator==(other)); }

template <typename T>
bool var::operator!=(const T& other) const {
  return !(operator==(other));
}

var var::operator[](val idx) const { return var(*this, idx); }

var var::operator[](var idx) const { return var(*this, idx.get_value()); }

template <typename... TArgs>
result<void> var::operator()(TArgs... args) const {
  return invoke<void>(args...);
}

template <typename TOut, typename... TArgs>
result<TOut> var::invoke(TArgs... args) const {
  stack_guard g(L);
  if (dirty_is<TOut(TArgs...)>()) {
    val::push_all<TArgs...>(L, args...);
    return internal::caller<TOut>::pcall(L, sizeof...(TArgs));
  }
  throw exception("Tried to invoke non-function.", L);
}

template <typename TOut>
result<TOut> var::invoke() const {
  stack_guard g(L);
  if (dirty_is<TOut()>()) {
    return internal::caller<TOut>::pcall(L, 0);
  }
  throw exception("Tried to invoke non-function.", L);
}

error var::do_chunk(const std::string& str) {
  stack_guard g(L);
  push_parent_key();
  auto err = luaL_loadstring(L, str.c_str());
  if (err != 0) {
    return error(err, "Unable to load chunk.", L);
  }
  err = lua_pcall(L, 0, 1, 0);
  if (err != 0) {
    return error(err, "Unable to run chunk.", L);
  }
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return error();
}

error var::do_file(const std::string& path) {
  stack_guard g(L);
  push_parent_key();
  auto err = luaL_loadfile(L, path.c_str());
  if (err != 0) {
    return error(err, "Unable to load file.", L);
  }
  err = lua_pcall(L, 0, 1, 0);
  if (err != 0) {
    return error(err, "Unable to run file.", L);
  }
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return error();
}

var_iterator var::begin() { return var_iterator(*this); }
const_var_iterator var::begin() const { return const_var_iterator(*this); }
var_iterator var::end() { return var_iterator(); }
const_var_iterator var::end() const { return const_var_iterator(); }

template <typename T, typename... TArgs>
ptr<T> var::create(TArgs... args) {
  stack_guard g(L);
  push_parent_key();
  do_create<T>::create(*this, args...);
  lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
  return get<ptr<T>>();
}

void var::push_parent_key() const {
  bool first = true;
  for (auto& l : lineage_) {
    l.push(L);
    if (&l != &lineage_.back()) {
      lua_gettable(L, first ? virtual_index_ : -2);
    }
    first = false;
  }
}

void var::push() const {
  push_parent_key();
  lua_gettable(L, lineage_.size() == 1 ? virtual_index_ : -2);
}

template <typename T>
bool var::dirty_is() const {
  push();
  return internal::typed_is<T>::is(L);
}

// Metastuff
template <typename T>
void var::setup_metatable() const {
  stack_guard g(L);
  {
    stack_guard g2(L, true);
    lua_getfield(L, LUA_REGISTRYINDEX, "metatables");
    if (lua_isnoneornil(L, -1)) {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_setfield(L, LUA_REGISTRYINDEX, "metatables");
      lua_getfield(L, LUA_REGISTRYINDEX, "metatables");
    }
    auto name = typeid(T).name();
    lua_getfield(L, -1, name);
    if (lua_isnoneornil(L, -1)) {
      lua_pop(L, 1);
      lua_newtable(L);
      userdata<T>::init_func(L);
      lua_setfield(L, -2, name);
      lua_getfield(L, -1, name);
    }
  }
  lua_setmetatable(L, -2);
}

// Private Constructors
var::var(lua_State* L, int virtual_index, val key)
    : L{L}, virtual_index_{virtual_index} {
  lineage_.push_back(key);
}

var::var(var v, val key)
    : L{v.L}, virtual_index_{v.virtual_index_}, lineage_{v.lineage_} {
  lineage_.push_back(key);
}
}