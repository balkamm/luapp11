#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/stack.hpp"
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

val::val() : type_{type::nil}, ptr{nullptr} {}
val::val(lua_Number n) : type_{type::number}, num{n} {}
val::val(int n) : type_{type::number}, num{(lua_Number)n} {}
val::val(bool b) : type_{type::boolean}, boolean{b} {}
val::val(const std::string& s) : type_{type::string}, str{s.c_str()} {}
val::val(const char* s) : type_{type::string}, str{s} {}

val::val(void* lud) : type_{type::lightuserdata}, ptr{lud} {}

val::val(std::initializer_list<std::pair<val, val>> t)
    : type_{type::table},
      table(new std::unordered_map<val, val, valueHasher>(t.begin(), t.end())) {
}

val::~val() {
  if (type_ == type::table) {
    table.reset();
  }
}

val::val(const val& other) : type_{other.type_} {
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

val::val(val&& other) { swap(*this, other); }

template <typename T>
T val::get() {
  switch (type_) {
    case type::number:
      return internal::get_number<T>::get(*this);
    case type::boolean:
      return internal::get_boolean<T>::get(*this);
    case type::string:
      return internal::get_string<T>::get(*this);
    case type::nil:
      return internal::get_nil<T>::get(*this);
    case type::table:
      return internal::get_table<T>::get(*this);
    case type::lightuserdata:
      return internal::get_lightuserdata<T>::get(*this);
    case type::userdata:
      return internal::get_userdata<T>::get(*this);
    case type::thread:
    default:
      throw luapp11::exception("Invalid Type Error");
  }
}

bool operator==(const val& a, const val& b) {
  if (a.type_ != b.type_) {
    return false;
  }
  switch (a.type_) {
    case val::type::number:
      return a.num == b.num;
    case val::type::boolean:
      return a.boolean == b.boolean;
    case val::type::string:
      return strcmp(a.str, b.str) == 0;

    case val::type::nil:
    case val::type::table:
    case val::type::thread:
    case val::type::lightuserdata:
      return a.ptr == b.ptr;
    default:
      luapp11::exception("Bad Type.");
  }
  return false;
}

bool operator!=(const val& a, const val& b) { return !(a == b); }

val& val::operator=(val other) {
  swap(*this, other);
  return *this;
}

void swap(val& a, val& b) {
  using std::swap;
  swap(a.type_, b.type_);
  switch (a.type_) {
    case val::type::nil:
    case val::type::lightuserdata:
      swap(a.ptr, b.ptr);
      break;
    case val::type::number:
      swap(a.num, b.num);
      break;
    case val::type::boolean:
      swap(a.boolean, b.boolean);
      break;
    case val::type::table:
      swap(a.table, b.table);
      break;
    case val::type::thread:
      swap(a.thread, b.thread);
      break;
    case val::type::string:
      swap(a.str, b.str);
      break;
    case val::type::none:
    case val::type::lua_function:
    case val::type::userdata:
      break;
  }
}

std::ostream& operator<<(std::ostream& out, const val& v) {
  switch (v.type_) {
    case val::type::nil:
      out << "nil:nil";
      break;
    case val::type::lightuserdata:
      out << "lightuserdata:" << v.ptr;
      break;
    case val::type::number:
      out << "number:" << v.num;
      break;
    case val::type::boolean:
      out << "boolean:" << v.boolean;
      break;
    case val::type::string:
      out << "string:" << v.str;
      break;
    case val::type::thread:
    case val::type::none:
    case val::type::table:
    case val::type::lua_function:
    case val::type::userdata:
      break;
  }
  return out;
}

static val nil() { return val(); }

// Private Constructors
val::val(lua_State* L, type t, int idx) : type_(t) {
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

val::val(lua_State* L, type t) : val(L, t, -1) {}
val::val(lua_State* L, int idx) : val(L, (type)lua_type(L, idx), idx) {}
val::val(lua_State* L) : val(L, (type)lua_type(L, -1), -1) {}

val::val(const std::string& str, type t) : type_{t}, str{str.c_str()} {}

// Puts on the top of the stack -0, +1, -
void val::push(lua_State* L) const {
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
}