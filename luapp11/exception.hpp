#pragma once

#include <exception>
#include <string>
#include <sstream>

namespace luapp11 {
class exception : public std::exception {
 public:
  const char* what() const noexcept override { return what_.c_str(); }

  const std::string& stack() const { return stack_; }

  friend std::ostream& operator<<(std::ostream& out, const exception& e) {
    return out << "luapp11 Exception: " << e.what() << std::endl << e.stack_
               << std::endl;
  }
 private:
  exception(std::string what) : what_(what), stack_() {}

  exception(std::string what, lua_State* L)
      : what_(what),
        stack_(stackdump(L)) {
  }

  static std::string stackdump(lua_State* L) {
    std::stringstream ss;
    ss << "Stack Dump:" << std::endl;

    const int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
      switch (lua_type(L, i)) {
        case LUA_TSTRING:
          ss << "string: " << lua_tostring(L, i) << std::endl;
          break;
        case LUA_TNUMBER:
          ss << "number: " << lua_tonumber(L, i) << std::endl;
          break;
        case LUA_TBOOLEAN:
          ss << "boolean: " << (lua_toboolean(L, i) ? "true" : "false")
             << std::endl;
          break;
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
          ss << "userdata: " << lua_topointer(L, i) << std::endl;
          break;
        case LUA_TFUNCTION:
          ss << "function" << std::endl;
          break;
        case LUA_TTABLE:
          ss << "table" << std::endl;
          break;
        default:
          ss << lua_typename(L, i) << std::endl;
          break;
      }
    }
    ss << std::endl;
    return ss.str();
  }

  std::string what_;
  std::string stack_;

  friend class error;
  friend class var;
  friend class val;
  friend class global;
  template <typename T> friend class result;
};

class error {
 public:
  ~error() = default;
  error(const error&) = default;

  enum class type {
    none = 0,
    runtime = LUA_ERRRUN,
    memory = LUA_ERRMEM,
    error = LUA_ERRERR,
    syntax = LUA_ERRSYNTAX
  };

  const type error_type() const { return type_; }

  const std::string& message() const { return message_; }
  const std::string& lua_message() const { return lua_message_; }

  const std::string& stack() const { return stack_; }

  const explicit operator bool() const { return type_ != type::none; }

  friend std::ostream& operator<<(std::ostream& out, const error& e) {
    return out << "luapp11 Error:" << (int)
           e.type_ << std::endl << e.message_ << std::endl << e.lua_message_
               << std::endl << e.stack_ << std::endl;
  }
 private:
  error() : type_ { type::none }
  {}
  error(int t, std::string message, lua_State* L) : type_ { (type) t }
  , message_ { message }
  {
    if (lua_isstring(L, -1)) {
      lua_message_ = lua_tostring(L, -1);
      lua_pop(L, 1);
    }
    stack_ = exception::stackdump(L);
  }

  type type_;
  std::string message_;
  std::string lua_message_;
  std::string stack_;

  friend class var;
  friend class val;
  friend class global;
  template <typename T> friend class result;
  friend error do_chunk(const std::string& str);
  friend error do_file(const std::string& path);
};
}