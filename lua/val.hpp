#pragma once

#include "lua/internal/stack_guard.hpp"
#include "lua/exception.hpp"
#include <memory>
#include <utility>
#include <unordered_map>
#include <cstring>
#include <sstream>

namespace lua {

class val {
 public:
  val() : type_ { type::nil }
  , ptr { nullptr }
  {}
  val(lua_Number n) : type_ { type::number }
  , num { n }
  {}
  val(int n) : type_ { type::number }
  , num { (lua_Number) n }
  {}
  val(bool b) : type_ { type::boolean }
  , boolean { b }
  {}
  val(const std::string& s) : type_ { type::string }
  , str { s.c_str() }
  {}
  val(const char* s) : type_ { type::string }
  , str { s }
  {}
  val(void* lud) : type_ { type::lightuserdata }
  , ptr { lud }
  {}
  val(std::initializer_list<std::pair<val, val>> t) : type_ { type::table }
  , table(new std::unordered_map<val, val, valueHasher>(t.begin(), t.end())) {}

  ~val() {
    if (type_ == type::table) {
      table.reset();
    }
  }

  val(const val& other) : type_ { other.type_ }
  {
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
      case type::chunk:
      case type::string:
        str = other.str;
        break;
      default:
        throw lua::exception("Bad type for copy.");
    }
  }

  val(val && other) { swap(*this, other); }

  template <typename T> T get() {
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
      case type::thread:
      default:
        throw lua::exception("Invalid Type Error");
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
        lua::exception("Bad Type.");
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
      case type::chunk:
      case type::string:
        swap(a.str, b.str);
        break;
      case type::none:
      case type::lua_function:
      case type::c_function:
        break;
    }
  }
 private:
  class valueHasher {
   public:
    size_t operator()(const val& v) const {
      std::hash<lua_Number> hasher;
      return hasher(v.num);
    }
  };
 public:
  static val nil() { return val(); }
  typedef std::unordered_map<val, val, valueHasher> table_type;
 private:
  enum class type : int {
    none = LUA_TNONE,
    nil = LUA_TNIL,
    number = LUA_TNUMBER,
    boolean = LUA_TBOOLEAN,
    string = LUA_TSTRING,
    table = LUA_TTABLE,
    lua_function = LUA_TFUNCTION,
    // Userdata = LUA_TUSERDATA,
        thread = LUA_TTHREAD,
        lightuserdata = LUA_TLIGHTUSERDATA,
        c_function = 10,
        chunk = 11,
  };

  val(lua_State* s, type t) : type_(t) {
    switch (t) {
      case type::number:
        num = lua_tonumber(s, -1);
        break;
      case type::boolean:
        boolean = lua_toboolean(s, -1);
        break;
      case type::string:
        str = lua_tostring(s, -1);
        break;
      case type::nil:
        ptr = nullptr;
        break;
      case type::table:
        throw lua::exception("not totally sure what to do...", s);
      case type::thread:
        thread = lua_tothread(s, -1);
        break;
      case type::lightuserdata:
        ptr = const_cast<void*>(lua_topointer(s, -1));
        break;
      default:
        throw lua::exception("Bad Type.", s);
    }
  }

  val(lua_State* s) : val(s, (type) lua_type(s, -1)) {}

  val(const std::string& str, type t) : type_ { t }
  , str { str.c_str() }
  {}

  // Puts on the top of the stack -0, +1, -
  virtual void push(lua_State* s) const {
    switch (type_) {
      case type::nil:
        lua_pushnil(s);
        break;
      case type::number:
        lua_pushnumber(s, num);
        break;
      case type::boolean:
        lua_pushboolean(s, boolean);
        break;
      case type::string:
        lua_pushstring(s, str);
        break;
      case type::table: {
        lua_newtable(s);
        for (auto p : *table) {
          p.first.push(s);
          p.second.push(s);
          lua_settable(s, -3);
        }
        break;
      }
        // case type::function: lua_pushcfunction(s, func); break;
        // case type::Userdata: {
        //  auto data = lua_newuserdata(s, userData.Size);
        //  std::copy(userData.Data, userData.Data[userData.Size], data);
        //  break;
        // }
      case type::thread:
        lua_pushthread(thread);
        break;
      case type::lightuserdata:
        lua_pushlightuserdata(s, ptr);
        break;
      case type::chunk:
        luaL_loadstring(s, str);
        break;
      default:
        throw lua::exception("Bad Type.", s);
    }
  }

  template <typename T, class Enable = void> struct get_number {
    static T get(const val& v) { throw lua::exception("Invalid Type Error"); }
  };

  template <typename T>
  struct get_number<
      T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
    static T get(const val& v) { return v.num; }
  };

  template <typename T>
  struct get_number<
      T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
    static T get(const val& v) {
      std::stringstream ss;
      ss << v.num;
      return ss.str();
    }
  };

  template <typename T, class Enable = void> struct get_boolean {
    static T get(const val& v) { throw lua::exception("Invalid Type Error"); }
  };

  template <typename T>
  struct get_boolean<
      T, typename std::enable_if<std::is_fundamental<T>::value>::type> {
    static T get(const val& v) { return v.boolean; }
  };

  template <typename T, class Enable = void> struct get_string {
    static T get(const val& v) { throw lua::exception("Invalid Type Error"); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
    static T get(const val& v) { return std::string(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, bool>::value>::type> {
    static T get(const val& v) { return true; }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, int>::value>::type> {
    static T get(const val& v) { return std::stoi(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, long>::value>::type> {
    static T get(const val& v) { return std::stol(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, long long>::value>::type> {
    static T get(const val& v) { return std::stoll(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, unsigned long>::value>::type> {
    static T get(const val& v) { return std::stoul(v.str); }
  };

  template <typename T>
  struct get_string<T,
                    typename std::enable_if<
                        std::is_same<T, unsigned long long>::value>::type> {
    static T get(const val& v) { return std::stoull(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, float>::value>::type> {
    static T get(const val& v) { return std::stof(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, double>::value>::type> {
    static T get(const val& v) { return std::stod(v.str); }
  };

  template <typename T>
  struct get_string<
      T, typename std::enable_if<std::is_same<T, long double>::value>::type> {
    static T get(const val& v) { return std::stold(v.str); }
  };

  template <typename T, class Enable = void> struct get_nil {
    static T get(const val& v) {
      throw lua::exception("get_nil: Invalid Type Error");
    }
  };

  template <typename T>
  struct get_nil<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
    static T get(const val& v) { return nullptr; }
  };

  template <typename T>
  struct get_nil<T,
                 typename std::enable_if<std::is_arithmetic<T>::value>::type> {
    static T get(const val& v) { return (T) NULL; }
  };

  template <typename T, class Enable = void> struct get_table {
    static T get(const val& v) {
      throw lua::exception("get_table: Invalid Type Error");
    }
  };

  template <typename T>
  struct get_table<
      T, typename std::enable_if<std::is_same<T, table_type>::value>::type> {
    static T get(const val& v) { return *v.table; }
  };

  template <typename T, class Enable = void> struct get_function {
    static T get(const val& v) { throw lua::exception("Invalid Type Error"); }
  };

  template <typename T>
  struct get_function<
      T, typename std::enable_if<std::is_function<T>::value>::type> {
    static T get(const val& v) { return dynamic_cast<T>(v.func); }
  };

  template <typename T> struct get_lightuserdata {
    static T get(const val& v) { return *(T*)v.ptr; }
  };
  template <typename T> struct get_lightuserdata<T*> {
    static T* get(const val& v) { return (T*)v.ptr; }
  };

  template <typename TArg, typename ... TArgs>
  static int push_all(lua_State* L, TArg a, TArgs ... args) {
    pusher<TArg>::push(L, a);
    return 1 + push_all(L, args ...);
  }

  template <typename TArg> static int push_all(lua_State* L, TArg a) {
    pusher<TArg>::push(L, a);
    return 1;
  }

  template <typename T, class Enable = void> struct pusher {
    static void push(lua_State* L, T v) { val(v).push(L); }
  };

  template <typename T>
  struct pusher<T,
                typename std::enable_if<std::is_member_function_pointer<
                    decltype(T::push)>::value>::type> {
    static void push(lua_State* L, T v) { v.push(L); }
  };

  template <typename T, class Enable = void> struct popper {
    static T pop(lua_State* L) { return val(L).get<T>(); }
  };

  template <typename T>
  struct popper<T, typename std::enable_if<std::is_same<T, val>::value>::type> {
    static val pop(lua_State* L) { return val(L); }
  };

  template <typename ... TArgs>
  struct popper<std::tuple<TArgs ...>, std::enable_if<true>> {
    static std::tuple<TArgs ...> pop(lua_State* L) {
      return std::tuple<TArgs ...>(popper<TArgs>(L) ...);
    }
  };

  struct UD {
    size_t Size;
    void* Data;
  };

  union {
    void* ptr;
    bool boolean;
    lua_Number num;
    const char* str;
    std::function<int(lua_State*)> func;
    lua_State* thread;
    // UD userData;
    std::shared_ptr<table_type> table;
  };

  type type_;
  friend class var;
  friend val chunk(const std::string& str);
};

inline val chunk(const std::string& str) { return val(str, val::type::chunk); }

}