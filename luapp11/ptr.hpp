#pragma once

namespace luapp11 {

template <typename T> class Ptr {
 public:
  T* get() { return *p_; }

  const T* get() const { return *p_; }

  T& operator->() { return **p_; }

  T& operator*() { return **p_; }

  bool expired() const { return *p_ == nullptr; }

 private:
  std::shared_ptr<T*> p_;
  lua_State* s_;

  Ptr(void* luaData, lua_State* s) : p_(reinterpret_cast<T*>(luaData)), s_(s) {
    push();
    lua_newtable(s_);
    lua_pushcfunction(s_, [this](lua_State * s) {
      return this->expire();
    });
    lua_setfield(s_, -2, "__gc");
    lua_setmetatable(s_, -2);
    lua_pop(s_, 1);
  }

  Ptr(lua_State* s) : Ptr(lua_topointer(s, -1), s) {}

  void push() {}

  bool expire() {
    p_.reset();
    return true;
  }
};

}
