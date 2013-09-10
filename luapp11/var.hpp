#pragma once

#include <type_traits>
#include <iostream>
#include <vector>

#include "luapp11/internal/traits.hpp"
#include "luapp11/ptr.hpp"

namespace luapp11 {

template <typename T> class userdata;

/**
 * A lua "variable".  A specific place in the lua environment which can be read from and assigned to.
 */
class var {
 public:
  var(const var& other) = default;
  var(var && other) = default;

  /**
   * Gets the value from this place in the lua environment.
   * @return The value found there.
   */
  val get_value() const {
    stack_guard g(L);
    push();
    return val(L);
  }

  /**
   * Gets the value from this place in the lua environment.
   * @typename T The type to get.
   * @return     The value found there.
   */
  template <typename T> T get() const { return get_value().get<T>(); }

  /**
    * Checks if the value at this place in the lua environment can be converted to the specified type.
    * @typename T The type to check.
    * @return     true if the value can be converted false otherwise.
    */
  template <typename T> bool is() const {
    stack_guard g(L);
    return dirty_is<T>();
  }

  /**
   * Gets the value from this place in the lua environment.  Returns the default value if unable to convert to the requested type.
   * @typename T        The type to get.
   * @param    fallback The value to return if type convertion fails.
   * @return            The value found there, or the default value if conversion fails.
   */
  template <typename T> T as(T && fallback) const {
    stack_guard g(L);
    return dirty_is<T>() ? val(L).get<T>() : fallback;
  }

  /**
   * Assigns the value at one place in thet lua environment to another.
   * @var    The location to assign from.
   * @return The location assigned to.
   */
  var& operator=(const var & var) {
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

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T> var& operator=(const T & toSet) {
    stack_guard g(L);
    push_parent_key();
    val::pusher<
        typename detail::convert_functor_to_std_function<T>::type>::push(L,
                                                                         toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T> var& operator=(const std::initializer_list<T> & toSet) {
    stack_guard g(L);
    push_parent_key();
    val::pusher<std::initializer_list<T>>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<val> & toSet) {
    stack_guard g(L);
    push_parent_key();
    val::pusher<std::initializer_list<val>>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<std::pair<val, val>> & toSet) {
    stack_guard g(L);
    push_parent_key();
    val::pusher<std::initializer_list<std::pair<val, val>>>::push(L, toSet);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  /**
   * Checks if two vars point to the same place in the lua environment.
   */
  bool operator==(const var& other) const {
    return L == other.L && virtual_index_ == other.virtual_index_ &&
           lineage_.size() == other.lineage_.size() &&
           std::mismatch(lineage_.begin(),
                         lineage_.end(),
                         other.lineage_.begin()).first == lineage_.end();
  }

  template <typename T> bool operator==(const T& other) const {
    return get<T>() == other;
  }

  bool operator!=(const var& other) const { return !(operator==(other)); }

  template <typename T> bool operator!=(const T& other) const {
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
  template <typename ... TArgs> result<void> operator()(TArgs ... args) const {
    return invoke<void>(args ...);
  }

  /**
   * Attempt to call the function at this location in the lua environment.  Can return a value.
   * @param args  The arguments to the call
   * @return      The result of the invocation.
   */
  template <typename TOut, typename ... TArgs>
  result<TOut> invoke(TArgs ... args) const {
    stack_guard g(L);
    if (dirty_is<TOut(TArgs ...)>()) {
      val::push_all<TArgs ...>(L, args ...);
      return caller<TOut>::pcall(L, sizeof ...(TArgs));
    }
    throw exception("Tried to invoke non-function.", L);
  }

  template <typename TOut> result<TOut> invoke() const {
    stack_guard g(L);
    if (dirty_is<TOut()>()) {
      return caller<TOut>::pcall(L, 0);
    }
    throw exception("Tried to invoke non-function.", L);
  }

  /**
   * Execute a string as lua.  Assigns it's return value to this location in the lua environment.
   * @param  str The lua code to execute.
   * @return     The error (if any) which occured while executing.
   */
  error do_chunk(const std::string& str) {
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

  /**
   * Execute a lua file.  Assigns it's return value to this location in the lua environment.
   * @param  path The location of the file on disk.
   * @return      The error (if any) which occured while executing.
   */
  error do_file(const std::string& path) {
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

  template <typename T, typename ... TArgs> ptr<T> create(TArgs ... args) {
    stack_guard g(L);
    push_parent_key();
    do_create<T>::create(*this, args ...);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return get<ptr<T>>();
  }

  class var_iterator :
      public std::iterator<std::forward_iterator_tag, std::pair<val, var>> {
   public:
    var_iterator(const var& v) : v_(&v), L(v.L), g_(new stack_guard(L)) {
      v.push();
      idx_ = lua_gettop(L);
      lua_pushnil(L);
      inc();
    }
    var_iterator(const var_iterator&) = delete;
    var_iterator(var_iterator && other) { swap(*this, other); }
    var_iterator& operator++() {
      inc();
      return *this;
    }
    bool operator==(const var_iterator& other) {
      return std::tie(L, idx_) == std::tie(other.L, other.idx_);
    }
    bool operator!=(const var_iterator& other) {
      return std::tie(L, idx_) == std::tie(other.L, other.idx_);
    }
    var_iterator& operator=(var_iterator other) {
      swap(*this,other);
      return*this;
    }
    std::pair<val, var> operator*() {
      val v(L);
      return std::make_pair(v, (*v_)[v]);
    }
    friend void swap(var_iterator& a, var_iterator& b)  // nothrow
        {
      using std::swap;

      swap(a.v_, b.v_);
      swap(a.L, b.L);
      swap(a.idx_, b.idx_);
      swap(a.g_, b.g_);
    }
   private:
    var_iterator(lua_State* L, int idx) : L(L), idx_(idx), v_(nullptr) {}
    void inc() {
      if (!lua_next(L, idx_)) {
        idx_ = 0;
      }
    }

    const var* v_;
    lua_State* L;
    int idx_;
    std::unique_ptr<stack_guard> g_;
    friend class var;
  };

  var_iterator begin() { return var_iterator(*this); }
  var_iterator end() { return var_iterator(L, 0); }
 private:

  // Pushing
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

  // Is checking
  template <typename T> bool dirty_is() const {
    push();
    return typed_is<T>::is(L);
  }

  template <typename T, class Enable = void> struct typed_is {
    static inline bool is(lua_State* L) { return false; }
  };

  template <typename T>
  struct typed_is<T,
                  typename std::enable_if<std::is_arithmetic<T>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) &&
             (lua_isboolean(L, -1) || lua_isnumber(L, -1));
    }
  };

  template <typename T>
  struct typed_is<
      T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) && lua_isstring(L, -1);
    }
  };

  template <typename T>
  struct typed_is<
      T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) && lua_isstring(L, -1);
    }
  };

  template <typename T>
  struct typed_is<T,
                  typename std::enable_if<std::is_function<T>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) && lua_isfunction(L, -1);
    }
  };

  template <typename T>
  struct typed_is<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) && lua_islightuserdata(L, -1);
    }
  };

  template <typename T>
  struct typed_is<
      T,
      typename std::enable_if<std::is_base_of<userdata<T>, T>::value>::type> {
    static inline bool is(lua_State* L) {
      return !lua_isnoneornil(L, -1) && lua_isuserdata(L, -1) &&
             userdata<T>::is(lua_touserdata(L, -1));
    }
  };

  // Invoking
  template <typename T, class Enable = void> struct caller {
    static result<T> call(lua_State* L, int nargs) {
      lua_call(L, nargs, 1);
      return val::popper<T>::get(L, -1);
    }
    static result<T> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, 1, 0);
      if (err) {
        return error(err, "Error calling lua method.", L);
      }
      return val::popper<T>::get(L, -1);
    }
  };

  template <typename T>
  struct caller<T, typename std::enable_if<std::is_void<T>::value>::type> {
    static result<T> call(lua_State* L, int nargs) {
      lua_call(L, nargs, 1);
      return result<T>();
    }
    static result<T> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, 1, 0);
      if (err) {
        return error(err, "Error calling lua method", L);
      }
      return result<T>();
    }
  };

  template <typename ... TArgs>
  struct caller<std::tuple<TArgs ...>, std::enable_if<true>::type> {
    static result<std::tuple<TArgs ...>> call(lua_State* L, int nargs) {
      lua_call(L, nargs, sizeof ...(TArgs));
      return val::popper<std::tuple<TArgs ...>>::get(
          L, (int) sizeof ...(TArgs) * -1);
    }
    static result<std::tuple<TArgs ...>> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, sizeof ...(TArgs), 0);
      if (err) {
        return error(err, "Error calling lua method.", L);
      }
      return val::popper<std::tuple<TArgs ...>>::get(
          L, (int) sizeof ...(TArgs) * -1);
    }
  };

  // Creators
  template <typename T, class Enable = void> struct do_create {
  };

  template <typename T>
  struct do_create<
      T,
      typename std::enable_if<std::is_base_of<userdata<T>, T>::value>::type> {
    template <typename ... TArgs>
    static void create(const var& v, TArgs ... args) {
      auto ptr = lua_newuserdata(v.L, sizeof(T));
      new (ptr) T(args ...);
      v.setup_metatable<T>();
    }
  };

  // Metastuff
  template <typename T> void setup_metatable() const {
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
  var(lua_State* L, int virtual_index, val key) : L { L }
  , virtual_index_ { virtual_index }
  { lineage_.push_back(key); }

  var(var v, val key) : L { v.L }
  , virtual_index_ { v.virtual_index_ }
  , lineage_ { v.lineage_ }
  { lineage_.push_back(key); }

  lua_State* L;
  std::vector<val> lineage_;
  int virtual_index_;

  friend class global;
  template <typename T> friend class userdata;
};

}