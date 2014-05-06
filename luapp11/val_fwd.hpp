#pragma once

#include "luapp11/fwd.hpp"
#include <unordered_map>
#include <utility>

namespace luapp11 {
class val {
 public:
  val();
  val(lua_Number n);
  val(int n);
  val(bool b);
  val(const std::string& s);
  val(const char* s);

  val(void* lud);
  val(std::initializer_list<std::pair<val, val>> t);

  ~val();

  val(const val& other);

  val(val&& other);

  template <typename T>
  T get();

  friend bool operator==(const val& a, const val& b);
  friend bool operator!=(const val& a, const val& b);
  val& operator=(val other);

  friend void swap(val& a, val& b);

  friend std::ostream& operator<<(std::ostream& out, const val& v);

  static val nil();

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
  val(lua_State* L, type t, int idx);

  val(lua_State* L, type t);
  val(lua_State* L, int idx);
  explicit val(lua_State* L);

  val(const std::string& str, type t);

  // Puts on the top of the stack -0, +1, -
  virtual void push(lua_State* L) const;

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
  friend class internal::core_access;
};
}