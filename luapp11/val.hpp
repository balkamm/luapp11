#pragma once

#include "luapp11/internal/stack_guard.hpp"
#include "luapp11/exception.hpp"
#include <memory>
#include <utility>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <cstring>
#include <sstream>

namespace luapp11 {

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
      case type::string:
        str = other.str;
        break;
      default:
        throw luapp11::exception("Bad type for copy.");
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
      default:
        throw luapp11::exception("Bad Type.", L);
    }
  }

  val(lua_State* L, type t) : val(L, t, -1) {}
  val(lua_State* L, int idx) : val(L, (type) lua_type(L, idx), idx) {}
  val(lua_State* L) : val(L, (type) lua_type(L, -1), -1) {}

  val(const std::string& str, type t) : type_ { t }
  , str { str.c_str() }
  {}

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
        for (auto p : *table) {
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

  // Getting
  template <typename T, class Enable = void> struct get_number {
    static T get(const val& v) {
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a number");
    }
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
    static T get(const val& v) {
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a boolean");
    }
  };

  template <typename T>
  struct get_boolean<
      T, typename std::enable_if<std::is_fundamental<T>::value>::type> {
    static T get(const val& v) { return v.boolean; }
  };

  template <typename T, class Enable = void> struct get_string {
    static T get(const val& v) {
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a string");
    }
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
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a nil");
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
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a table");
    }
  };

  template <typename T>
  struct get_table<
      T, typename std::enable_if<std::is_same<T, table_type>::value>::type> {
    static T get(const val& v) { return *v.table; }
  };

  template <typename T, class Enable = void> struct get_function {
    static T get(const val& v) {
      throw luapp11::exception(std::string("Invalid Type Error: ") +
                               typeid(T).name() + " not a function");
    }
  };

  // template <typename T>
  // struct get_function<
  //     T, typename std::enable_if<std::is_function<T>::value>::type> {
  //   static T get(const val& v) { return dynamic_cast<T>(v.func); }
  // };

  template <typename T> struct get_lightuserdata {
    static T get(const val& v) { return *(T*)v.ptr; }
  };
  template <typename T> struct get_lightuserdata<T*> {
    static T* get(const val& v) { return (T*)v.ptr; }
  };

  template <typename T, class Enable = void> struct popper {
    static T get(lua_State* L, int idx = -1) { return val(L, idx).get<T>(); }
  };

  // Popping
  template <typename T>
  struct popper<T, typename std::enable_if<std::is_same<T, val>::value>::type> {
    static val get(lua_State* L, int idx = -1) { return val(L, idx); }
  };

  struct stack_popper {
    stack_popper(int start) : idx { start }
    {}

    template <typename T> T get(lua_State* L) {
      auto ret = val::popper<T>::get(L, idx);
      idx++;
      return ret;
    }

   private:
    int idx;
  };

  template <typename ... TArgs>
  struct popper<std::tuple<TArgs ...>, std::enable_if<true>::type> {
    static std::tuple<TArgs ...> get(lua_State* L, int idx) {
      stack_popper p(idx);
      return std::tuple<TArgs ...>(p.get<TArgs>(L) ...);
    }
  };

  // Counting
  template <typename T, class Enable = void> struct counter {
    static int count() { return 1; }
  };

  template <typename T>
  struct counter<T,
                 typename std::enable_if<std::is_same<T, void>::value>::type> {
    static int count() { return 0; }
  };

  template <typename ... TArgs>
  struct counter<std::tuple<TArgs ...>, std::enable_if<true>::type> {
    static int count() { return sizeof ...(TArgs); }
  };

  // Pushing
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
  };

  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };

  template <typename T>
  struct pusher<T,
                typename std::enable_if<std::is_same<T, char>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, char16_t>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, char32_t>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, wchar_t>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, signed char>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, short int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<T, typename std::enable_if<std::is_same<T, int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, long int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, long long int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, unsigned char>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<T,
                typename std::enable_if<
                    std::is_same<T, unsigned short int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, unsigned int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<T,
                typename std::enable_if<
                    std::is_same<T, unsigned long int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };
  template <typename T>
  struct pusher<T,
                typename std::enable_if<
                    std::is_same<T, unsigned long long int>::value>::type> {
    static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
  };

  template <typename T>
  struct pusher<T,
                typename std::enable_if<std::is_same<T, bool>::value>::type> {
    static void push(lua_State* L, const T& b) { lua_pushboolean(L, b); }
  };

  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
    static void push(lua_State* L, const T& str) { lua_pushstring(L, str); }
  };

  template <typename T>
  struct pusher<
      T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
    static void push(lua_State* L, const T& str) {
      lua_pushstring(L, str.c_str());
    }
  };

  template <typename T>
  struct pusher<T, typename std::enable_if<std::is_same<T, val>::value>::type> {
    static void push(lua_State* L, const T& v) { v.push(L); }
  };

  template <typename TRet, typename ... TArgs>
  struct pusher<std::function<TRet(TArgs ...)>, std::enable_if<true>::type> {
    typedef std::function<TRet(TArgs ...)> f_type;
    static int call(lua_State* L) {
      int nargs = lua_gettop(L);
      if (nargs != sizeof ...(TArgs)) {
        pusher<const char*>::push(
            L, "C++ function invoked with the wrong number of arguments.");
        lua_error(L);
      }
      void* ptr = lua_touserdata(L, lua_upvalueindex(1));
      auto func = *(f_type*)ptr;
      stack_popper p(-nargs);
      try {
        TRet ret = func(p.get<TArgs>(L) ...);
        pusher<TRet>::push(L, ret);
      }
      catch (std::exception e) {
        pusher<const char*>::push(L, e.what());
        lua_error(L);
      }
      return counter<TRet>::count();
    }

    static void push(lua_State* L, f_type* func) {
      lua_pushlightuserdata(L, func);
      lua_pushcclosure(L, &call, 1);
    }

    static int deleter(lua_State* L) {
      void* ptr = lua_touserdata(L, -1);
      auto func = *static_cast<f_type*>(ptr);
      func.~f_type();
      return 0;
    }

    static void push(lua_State* L, const f_type& func) {
      void* f_data = lua_newuserdata(L, sizeof(f_type));
      f_type* f = new (f_data) f_type(func);
      lua_newtable(L);
      lua_pushcfunction(L, &deleter);
      lua_setfield(L, -2, "__gc");
      lua_setmetatable(L, -2);

      lua_pushcclosure(L, &call, 1);
    }
  };

  template <typename TRet, typename ... TArgs>
  struct pusher<TRet(*)(TArgs ...), std::enable_if<true>::type> {
    typedef TRet(*f_type)(TArgs ...);
    static int call(lua_State* L) {
      int nargs = lua_gettop(L);
      if (nargs != sizeof ...(TArgs)) {
        pusher<const char*>::push(
            L, "C++ function invoked with the wrong number of arguments.");
        lua_error(L);
      }
      void* ptr = lua_touserdata(L, lua_upvalueindex(1));
      auto func = (f_type) ptr;
      stack_popper p(-nargs);
      try {
        TRet ret = func(p.get<TArgs>(L) ...);
        pusher<TRet>::push(L, ret);
      }
      catch (std::exception e) {
        pusher<const char*>::push(L, e.what());
        lua_error(L);
      }
      return counter<TRet>::count();
    }

    static void push(lua_State* L, f_type func) {
      lua_pushlightuserdata(L, (void*)func);
      lua_pushcclosure(L, &call, 1);
    }
  };

  template <typename TFrom, typename TTo>
  struct pusher<std::map<TFrom, TTo>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::map<TFrom, TTo>& map) {
      lua_newtable(L);
      for (auto& i : map) {
        pusher<TFrom>::push(L, i.first);
        pusher<TTo>::push(L, i.second);
        lua_settable(L, -3);
      }
    }
  };

  template <typename TFrom, typename TTo>
  struct pusher<std::unordered_map<TFrom, TTo>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::unordered_map<TFrom, TTo>& map) {
      lua_newtable(L);
      for (auto& i : map) {
        pusher<TFrom>::push(L, i.first);
        pusher<TTo>::push(L, i.second);
        lua_settable(L, -3);
      }
    }
  };

  template <typename TFrom, typename TTo>
  struct pusher<std::initializer_list<std::pair<TFrom, TTo>>,
                std::enable_if<true>::type> {
    static void push(
        lua_State* L,
        const std::initializer_list<std::pair<TFrom, TTo>>& map) {
      lua_newtable(L);
      for (auto& i : map) {
        pusher<TFrom>::push(L, i.first);
        pusher<TTo>::push(L, i.second);
        lua_settable(L, -3);
      }
    }
  };

  template <typename T>
  struct pusher<std::vector<T>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::vector<T>& vec) {
      lua_newtable(L);
      int idx = 0;
      for (auto& i : vec) {
        lua_pushnumber(L, ++idx);
        pusher<T>::push(L, i);
        lua_settable(L, -3);
      }
    }
  };

  template <typename T>
  struct pusher<std::initializer_list<T>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::initializer_list<T>& vec) {
      lua_newtable(L);
      int idx = 0;
      for (auto& i : vec) {
        lua_pushnumber(L, ++idx);
        pusher<T>::push(L, i);
        lua_settable(L, -3);
      }
    }
  };

  template <typename T> struct pusher<std::set<T>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::set<T>& set) {
      lua_newtable(L);
      int idx = 0;
      for (auto& i : set) {
        lua_pushnumber(L, ++idx);
        pusher<T>::push(L, i);
        lua_settable(L, -3);
      }
    }
  };

  template <typename T>
  struct pusher<std::unordered_set<T>, std::enable_if<true>::type> {
    static void push(lua_State* L, const std::unordered_set<T>& set) {
      lua_newtable(L);
      int idx = 0;
      for (auto& i : set) {
        lua_pushnumber(L, ++idx);
        pusher<T>::push(L, i);
        lua_settable(L, -3);
      }
    }
  };

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
  friend class var;
  friend val chunk(const std::string& str);
};

}