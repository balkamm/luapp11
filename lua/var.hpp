#pragma once

#include <type_traits>
#include <iostream>

namespace lua {

class var {
 public:
  var(const var& other) = default;
  var(var && other) = default;

  // Gets the value of the var
  val get_value() const {
    stack_guard g(L);
    push();
    return val(L);
  }

  // Gets the value of the var typed
  template <typename T> T get() const { return get_value().get<T>(); }

  template <typename T> bool is() const {
    stack_guard g(L);
    return dirty_is<T>();
  }

  template <typename T> T as(T && fallback) {
    stack_guard g(L);
    return dirty_is<T>() ? val(L).get<T>() : fallback;
  }

  // Assigns the var with another var
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

  var& operator=(const val & val) {
    stack_guard g(L);
    push_parent_key();
    val.push(L);
    lua_settable(L, lineage_.size() == 1 ? virtual_index_ : -3);
    return *this;
  }

  bool operator==(const var& other) {
    return L == other.L && virtual_index_ == other.virtual_index_ &&
           lineage_.size() == other.lineage_.size() &&
           std::mismatch(lineage_.begin(),
                         lineage_.end(),
                         other.lineage_.begin()).first == lineage_.end();
  }

  var operator[](val idx) { return var(*this, idx); }

  var operator[](var idx) { return var(*this, idx.get_value()); }

  template <typename ... TArgs> result<void> operator()(TArgs ... args) {
    return invoke<void>(args ...);
  }

  template <typename TOut, typename ... TArgs>
  result<TOut> invoke(TArgs ... args) {
    if (dirty_is<TOut(TArgs ...)>()) {
      val::push_all<TArgs ...>(L, args ...);
      return caller<TOut>::call(L, sizeof ...(TArgs));
    }
    throw exception("Tried to invoke non-function.");
  }

  void do_chunk(const std::string& str) {
    luaL_loadstring(L, str.c_str());
    int idx = lua_gettop(L);
    {
      stack_guard g(L);
      push();
      if (lua_istable(L, -1)) {
        lua_setfenv(L, idx);
      }
    }
    lua_call(L, 0, LUA_MULTRET);
  }

 protected:
  void push_parent_key() const {
    bool first;
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

  template <typename T> bool dirty_is() const {
    push();
    return typed_is<T>::is(L);
  }

  template <typename T, class Enable = void> struct caller {
    static result<T> call(lua_State* L, int nargs) {
      lua_call(L, nargs, 1);
      return val::popper<T>::pop(L);
    }
    static result<T> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, 1, 0);
      if (err) {
        return error((error::type) err, "Error calling lua method.", L);
      }
      return val::popper<T>::pop(L);
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
        return error((error::type) err, "Error calling lua method", L);
      }
      return result<T>();
    }
  };

  template <typename ... TArgs>
  struct caller<std::tuple<TArgs ...>, std::enable_if<true>::type> {
    static result<std::tuple<TArgs ...>> call(lua_State* L, int nargs) {
      lua_call(L, nargs, sizeof ...(TArgs));
      return result<std::tuple<TArgs ...>>();
    }
    static result<std::tuple<TArgs ...>> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, sizeof ...(TArgs), 0);
      if (err) {
        return error((error::type) err, "Error calling lua method", L);
      }
      return result<std::tuple<TArgs ...>>();
    }
  };

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

  friend class root;
};

}