#pragma once

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstring>

namespace luapp11 {
class val {
 public:
  val() : type_{type::nil}, ptr{nullptr} {}
  val(lua_Number n) : type_{type::number}, num{n} {}
  val(int n) : type_{type::number}, num{(lua_Number)n} {}
  val(bool b) : type_{type::boolean}, boolean{b} {}
  val(const std::string& s) : type_{type::string}, str{s.c_str()} {}
  val(const char* s) : type_{type::string}, str{s} {}

  val(void* lud) : type_{type::lightuserdata}, ptr{lud} {}

  val(std::initializer_list<std::pair<val, val>> t)
      : type_{type::table},
        table(new std::unordered_map<val, val, valueHasher>(t.begin(),
                                                            t.end())) {}

  ~val() {
    if (type_ == type::table) {
      table.reset();
    }
  }

  val(const val& other) : type_{other.type_} {
    switch (type_) {
      case type::nil:
      case type::lightuserdata:
        ptr = other.ptr;
        break;
      case type::number:
        num = other.num;
        break;
      case type::boolean:
        boolean = other.boolean;
        break;
      case type::table:
        table = other.table;
        break;
      case type::thread:
        thread = other.thread;
        break;
      case type::string:
        str = other.str;
        break;
      default:
        throw luapp11::exception("Bad type for copy.");
    }
  }

  val(val&& other) { swap(*this, other); }

  template <typename T>
  T get() {
    switch (type_) {
      case type::number:
        return get_number<T>::get(*this);
      case type::boolean:
        return get_boolean<T>::get(*this);
      case type::string:
        return get_string<T>::get(*this);
      case type::nil:
        return get_nil<T>::get(*this);
      case type::table:
        return get_table<T>::get(*this);
      case type::lightuserdata:
        return get_lightuserdata<T>::get(*this);
      case type::userdata:
        return get_userdata<T>::get(*this);
      case type::thread:
      default:
        throw luapp11::exception("Invalid Type Error");
    }
  }

  friend bool operator==(const val& a, const val& b) {
    if (a.type_ != b.type_) {
      return false;
    }
    switch (a.type_) {
      case type::number:
        return a.num == b.num;
      case type::boolean:
        return a.boolean == b.boolean;
      case type::string:
        return strcmp(a.str, b.str) == 0;

      case type::nil:
      case type::table:
      case type::thread:
      case type::lightuserdata:
        return a.ptr == b.ptr;
      default:
        luapp11::exception("Bad Type.");
    }
    return false;
  }

  friend bool operator!=(const val& a, const val& b) { return !(a == b); }

  val& operator=(val other) {
    swap(*this, other);
    return *this;
  }

  friend void swap(val& a, val& b) {
    using std::swap;
    swap(a.type_, b.type_);
    switch (a.type_) {
      case type::nil:
      case type::lightuserdata:
        swap(a.ptr, b.ptr);
        break;
      case type::number:
        swap(a.num, b.num);
        break;
      case type::boolean:
        swap(a.boolean, b.boolean);
        break;
      case type::table:
        swap(a.table, b.table);
        break;
      case type::thread:
        swap(a.thread, b.thread);
        break;
      case type::string:
        swap(a.str, b.str);
        break;
      case type::none:
      case type::lua_function:
      case type::userdata:
        break;
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const val& v) {
    switch (v.type_) {
      case type::nil:
        out << "nil:nil";
        break;
      case type::lightuserdata:
        out << "lightuserdata:" << v.ptr;
        break;
      case type::number:
        out << "number:" << v.num;
        break;
      case type::boolean:
        out << "boolean:" << v.boolean;
        break;
      case type::string:
        out << "string:" << v.str;
        break;
      case type::thread:
      case type::none:
      case type::table:
      case type::lua_function:
      case type::userdata:
        break;
    }
    return out;
  }

  static val nil() { return val(); }

 private:
  class valueHasher {
   public:
    size_t operator()(const val& v) const {
      std::hash<lua_Number> hasher;
      return hasher(v.num);
    }
  };

 public:
  typedef std::unordered_map<val, val, valueHasher> table_type;
  typedef std::function<lua_CFunction> function_type;

 private:
#include "luapp11/internal/stack.hpp"
  enum class type : int {
    none = LUA_TNONE,
    nil = LUA_TNIL,
    number = LUA_TNUMBER,
    boolean = LUA_TBOOLEAN,
    string = LUA_TSTRING,
    table = LUA_TTABLE,
    lua_function = LUA_TFUNCTION,
    userdata = LUA_TUSERDATA,
    thread = LUA_TTHREAD,
    lightuserdata = LUA_TLIGHTUSERDATA,
  };

  // Private Constructors
  val(lua_State* L, type t, int idx) : type_(t) {
    switch (t) {
      case type::number:
        num = lua_tonumber(L, idx);
        break;
      case type::boolean:
        boolean = lua_toboolean(L, idx);
        break;
      case type::string:
        str = lua_tostring(L, idx);
        break;
      case type::nil:
        ptr = nullptr;
        break;
      case type::table:
        throw luapp11::exception("not totally sure what to do...", L);
      case type::thread:
        thread = lua_tothread(L, idx);
        break;
      case type::lightuserdata:
        ptr = const_cast<void*>(lua_topointer(L, idx));
        break;
      case type::userdata:
        ptr = lua_touserdata(L, idx);
        break;

      default:
        throw luapp11::exception("Bad Type.", L);
    }
  }

  val(lua_State* L, type t) : val(L, t, -1) {}
  val(lua_State* L, int idx) : val(L, (type)lua_type(L, idx), idx) {}
  explicit val(lua_State* L) : val(L, (type)lua_type(L, -1), -1) {}

  val(const std::string& str, type t) : type_{t}, str{str.c_str()} {}

  // Puts on the top of the stack -0, +1, -
  virtual void push(lua_State* L) const {
    switch (type_) {
      case type::nil:
        lua_pushnil(L);
        break;
      case type::number:
        lua_pushnumber(L, num);
        break;
      case type::boolean:
        lua_pushboolean(L, boolean);
        break;
      case type::string:
        lua_pushstring(L, str);
        break;
      case type::table: {
        lua_newtable(L);
        for (auto& p : *table) {
          p.first.push(L);
          p.second.push(L);
          lua_settable(L, -3);
        }
        break;
      }
      // case type::cfunction: {
      //   lua_pushcfunction(s, func);
      //   break;
      // }
      // case type::Userdata: {
      //  auto data = lua_newuserdata(s, userData.Size);
      //  std::copy(userData.Data, userData.Data[userData.Size], data);
      //  break;
      // }
      case type::thread:
        lua_pushthread(thread);
        break;
      case type::lightuserdata:
        lua_pushlightuserdata(L, ptr);
        break;
      default:
        throw luapp11::exception("Bad Type.", L);
    }
  }

  // Member variables
  struct UD {
    size_t Size;
    void* Data;
  };

  union {
    void* ptr;
    bool boolean;
    lua_Number num;
    const char* str;
    // function_type func;
    lua_State* thread;
    // UD userData;
    std::shared_ptr<table_type> table;
  };

  type type_;
  friend class stack_var;
  friend class var;
};
}
