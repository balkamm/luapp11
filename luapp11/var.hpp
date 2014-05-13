#pragma once

#include <type_traits>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include "luapp11/internal/traits.hpp"
#include "luapp11/internal/type_registry.hpp"
#include "luapp11/ptr.hpp"

namespace luapp11 {

class var {
 public:
  var(const var& other) = default;
  var(var&& other) = default;

  /**
   * Pushes this variable onto the stack.
   * @return a stack_var which refers to the value on the stack.
   */
  stack_var localize() const {
    internal::stack_guard g(L);
    push();
    return stack_var(L, lua_gettop(L), std::move(g));
  }

  /**
   * Gets the value from this place in the lua environment.
   * @return The value found there.
   */
  val get_value() const { return localize().get_value(); }

  /**
   * Gets the value from this place in the lua environment.
   * @typename T The type to get.
   * @return     The value found there.
   */
  template <typename T>
  T get() const {
    return get_value().get<T>();
  }

  /**
   * Checks if the value at this place in the lua environment can be converted
   * to the specified type.
   * @typename T The type to check.
   * @return     true if the value can be converted false otherwise.
   */
  template <typename T>
  bool is() const {
    return localize().is<T>();
  }

  bool is_table() const { return localize().is_table(); }

  /**
   * Gets the value from this place in the lua environment.  Returns the default
   * value if unable to convert to the requested type.
   * @typename T        The type to get.
   * @param    fallback The value to return if type convertion fails.
   * @return            The value found there, or the default value if
   * conversion fails.
   */
  template <typename T>
  T as(T&& fallback) const {
    return localize().as<T>(std::forward<T>(fallback));
  }

  /**
   * Assigns the value at one place in thet lua environment to another.
   * @var    The location to assign from.
   * @return The location assigned to.
   */
  var& operator=(const var& var) {
    internal::stack_guard g(L);
    push_parent_key();
    if (L == var.L) {
      internal::stack_guard g2(L, true);
      var.push();
    } else {
      var.get_value().push(L);
    }
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  var& operator=(const T& toSet) {
    internal::stack_guard g(L);
    push_parent_key();
    internal::pusher<typename internal::convert_functor_to_std_function<
        T>::type>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  var& operator=(const std::initializer_list<T>& toSet) {
    internal::stack_guard g(L);
    push_parent_key();
    internal::pusher<std::initializer_list<T>>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<val>& toSet) {
    internal::stack_guard g(L);
    push_parent_key();
    internal::pusher<std::initializer_list<val>>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<std::pair<val, val>>& toSet) {
    internal::stack_guard g(L);
    push_parent_key();
    internal::pusher<std::initializer_list<std::pair<val, val>>>::push(L,
                                                                       toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Checks if two vars point to the same place in the lua environment.
   */
  bool operator==(const var& other) const {
    return L == other.L && virtual_index_ == other.virtual_index_ &&
           lineage_.size() == other.lineage_.size() &&
           std::mismatch(lineage_.begin(), lineage_.end(),
                         other.lineage_.begin()).first == lineage_.end();
  }

  template <typename T>
  bool operator==(const T& other) const {
    return get<T>() == other;
  }

  template <typename T>
  bool operator!=(const T& other) const {
    return !(operator==(other));
  }

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The index to get.
   * @return      A var which points to the child of this location at index idx.
   */
  var operator[](val idx) const { return var(*this, idx); }
  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The location of the index to get.
   * @return      A var which points to the child of this location at index idx.
   */
  var operator[](var idx) const { return var(*this, idx.get_value()); }

  /**
   * Attempt to call the function at this location in the lua environment.
   * @param args  The arguments to the call
   */
  template <typename... TArgs>
  result<void> operator()(TArgs... args) const {
    return localize()(std::forward<TArgs>(args)...);
  }

  /**
   * Attempt to call the function at this location in the lua environment.  Can
   * return a value.
   * @param args  The arguments to the call
   * @return      The result of the invocation.
   */
  template <typename TOut, typename... TArgs>
  result<TOut> invoke(TArgs... args) const {
    return localize().invoke<TOut>(std::forward<TArgs>(args)...);
  }

  template <typename TOut>
  result<TOut> invoke() const {
    return localize().invoke<TOut>();
  }

  /**
   * Execute a string as lua.  Assigns it's return value to this location in the
   * lua environment.
   * @param  str The lua code to execute.
   * @return     The error (if any) which occured while executing.
   */
  error do_chunk(const std::string& str) {
    internal::stack_guard g(L);
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

  /**
   * Execute a lua file.  Assigns it's return value to this location in the lua
   * environment.
   * @param  path The location of the file on disk.
   * @return      The error (if any) which occured while executing.
   */
  error do_file(const std::string& path) {
    internal::stack_guard g(L);
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

  template <typename T, typename... TArgs>
  ptr<T> create(TArgs... args) {
    internal::stack_guard g(L);
    push_parent_key();
    auto p = internal::create<T>(L, std::forward<TArgs>(args)...);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return ptr<T>((T*)p);
  }

 private:
  void push_parent_key() const {
    bool first = true;
    for (auto& l : lineage_) {
      l.push(L);
      if (&l != &lineage_.back()) {
        lua_gettable(L, first ? virtual_index_ : -2);
      }
      first = false;
    }
  }

  void push() const {
    push_parent_key();
    lua_gettable(L, lineage_.size() == 1 ? virtual_index_ : -2);
  }

  // Metastuff
  template <typename T>
  void setup_metatable() const {
    internal::stack_guard g(L);
    {
      internal::stack_guard g2(L, true);
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

  var(lua_State* L, int virtual_index, val key)
      : L{L}, virtual_index_{virtual_index} {
    lineage_.push_back(key);
  }

  var(var v, val key)
      : L{v.L}, virtual_index_{v.virtual_index_}, lineage_{v.lineage_} {
    lineage_.push_back(key);
  }

  lua_State* L;
  std::vector<val> lineage_;
  int virtual_index_;

  friend class global;
};
}

namespace std {
inline string to_string(const luapp11::var& v, string indent = "") {
  return std::to_string(v.localize(), indent);
}
}
