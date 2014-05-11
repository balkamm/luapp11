#pragma once

#include "luapp11/internal/traits.hpp"
#include "luapp11/internal/stack_guard.hpp"
#include <mutex>

namespace luapp11 {
namespace internal {
class type_registry {
 public:
  template <typename T>
  static std::string type_name() {
    return type_namer<T>::name();
  }

  template <typename T>
  static bool is(void* ptr) {
    return type_is<T>::is(ptr);
  }

  template <typename T>
  static T* cast(void* ptr) {
    return caster<T>::cast(ptr);
  }

  template <typename T>
  static bool known_type() {
    std::lock_guard<std::mutex> g(instance().m_);
    return instance().known_types_.count(typeid(T).hash_code());
  }

  template <typename T>
  static void register_type(lua_State* L,
                            std::function<void(lua_State*)> callback) {
    std::lock_guard<std::mutex> g(instance().m_);
    auto hash = typeid(T).hash_code();
    auto name = type_name<T>();
    if (!instance().known_types_.count(hash)) {
      stack_guard g(L, true);
      luaL_newmetatable(L, name.c_str());
      callback(L);
      instance().known_types_[hash] = name;
    } else {
      luaL_getmetatable(L, name.c_str());
    }
  }

 private:
  template <typename T, typename Enable = void>
  struct type_namer {
    static std::string name() { return typeid(T).name(); }
  };

  template <typename T>
  struct type_namer<T, typename std::enable_if<std::is_function<
                           decltype(&T::type_name)>::value>::type> {
    static std::string name() { return T::type_name(); }
  };

  template <typename T, typename Enable = void>
  struct type_is {
    static bool is(void*) { return true; }
  };

  template <typename T>
  struct type_is<T, typename std::enable_if<
                        std::is_function<decltype(&T::is)>::value>::type> {
    static bool is(void* ptr) { return T::is(ptr); }
  };

  template <typename T, typename Enable = void>
  struct caster {
    static T* cast(void* ptr) { return (T*)ptr; }
  };

  template <typename T>
  struct caster<T, typename std::enable_if<
                       std::is_function<decltype(&T::cast)>::value>::type> {
    static T* cast(void* ptr) { return T::cast(ptr); }
  };

  static type_registry& instance() {
    static type_registry inst;
    return inst;
  }

  std::map<size_t, std::string> known_types_;
  std::mutex m_;
};
}
}