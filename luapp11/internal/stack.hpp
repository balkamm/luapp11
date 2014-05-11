#pragma once

namespace luapp11 {
namespace internal {

// template <typename T>
// struct get_userdata {
//   static T get(const val& v) {
//     throw luapp11::exception(
//         std::string("Invalid Type Error: is a userdata, expected: ") +
//         typeid(T).name());
//   }
// };

// template <typename T>
// struct get_userdata<ptr<T>> {
//   static ptr<T> get(const val& v) {
//     return luapp11::ptr<T>(userdata<T>::cast(v.ptr));
//   }
// };

template <typename T, class Enable = void>
struct popper {
  static T get(lua_State* L, int idx = -1) { return val(L, idx).get<T>(); }
};

// Popping
template <typename T>
struct popper<T, typename std::enable_if<std::is_same<T, val>::value>::type> {
  static val get(lua_State* L, int idx = -1) { return val(L, idx); }
};

struct stack_popper {
  stack_popper(int start) : idx{start} {}

  template <typename T>
  T get(lua_State* L) {
    auto ret = popper<T>::get(L, idx);
    idx++;
    return ret;
  }

 private:
  int idx;
};

template <typename... TArgs>
struct popper<std::tuple<TArgs...>, std::enable_if<true>::type> {
  static std::tuple<TArgs...> get(lua_State* L, int idx) {
    stack_popper p(idx);
    return std::tuple<TArgs...>(p.get<TArgs>(L)...);
  }
};

// Counting
template <typename T, class Enable = void>
struct counter {
  static int count() { return 1; }
};

template <typename T>
struct counter<T, typename std::enable_if<std::is_same<T, void>::value>::type> {
  static int count() { return 0; }
};

template <typename... TArgs>
struct counter<std::tuple<TArgs...>, std::enable_if<true>::type> {
  static int count() { return sizeof...(TArgs); }
};

// Pushing
template <typename T, class Enable = void>
struct pusher {};

template <typename TArg, typename... TArgs>
static int push_all(lua_State* L, TArg a, TArgs... args) {
  pusher<TArg>::push(L, a);
  return 1 + push_all(L, args...);
}

template <typename TArg>
static int push_all(lua_State* L, TArg a) {
  pusher<TArg>::push(L, a);
  return 1;
}

template <typename T>
struct pusher<T,
              typename std::enable_if<std::is_floating_point<T>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};

template <typename T>
struct pusher<T, typename std::enable_if<std::is_same<T, char>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};
template <typename T>
struct pusher<T, typename std::enable_if<std::is_same<T, int>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};
template <typename T>
struct pusher<T,
              typename std::enable_if<std::is_same<T, long int>::value>::type> {
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
struct pusher<T, typename std::enable_if<
                     std::is_same<T, unsigned short int>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};
template <typename T>
struct pusher<
    T, typename std::enable_if<std::is_same<T, unsigned int>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};
template <typename T>
struct pusher<T, typename std::enable_if<
                     std::is_same<T, unsigned long int>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};
template <typename T>
struct pusher<T, typename std::enable_if<
                     std::is_same<T, unsigned long long int>::value>::type> {
  static void push(lua_State* L, const T& num) { lua_pushnumber(L, num); }
};

template <typename T>
struct pusher<T, typename std::enable_if<std::is_same<T, bool>::value>::type> {
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

template <typename TRet, typename... TArgs>
struct pusher<std::function<TRet(TArgs...)>, std::enable_if<true>::type> {
  typedef std::function<TRet(TArgs...)> f_type;
  static int call(lua_State* L) {
    int nargs = lua_gettop(L);
    if (nargs != sizeof...(TArgs)) {
      pusher<const char*>::push(
          L, "C++ function invoked with the wrong number of arguments.");
      lua_error(L);
    }
    void* ptr = lua_touserdata(L, lua_upvalueindex(1));
    auto func = *(f_type*)ptr;
    stack_popper p(-nargs);
    try {
      TRet ret = func(p.get<TArgs>(L)...);
      pusher<TRet>::push(L, ret);
    } catch (std::exception e) {
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

template <typename TRet, typename... TArgs>
struct pusher<TRet (*)(TArgs...), std::enable_if<true>::type> {
  typedef TRet (*f_type)(TArgs...);
  static int call(lua_State* L) {
    int nargs = lua_gettop(L);
    if (nargs != sizeof...(TArgs)) {
      pusher<const char*>::push(
          L, "C++ function invoked with the wrong number of arguments.");
      lua_error(L);
    }
    void* ptr = lua_touserdata(L, lua_upvalueindex(1));
    auto func = (f_type)ptr;
    stack_popper p(-nargs);
    try {
      TRet ret = func(p.get<TArgs>(L)...);
      pusher<TRet>::push(L, ret);
    } catch (std::exception e) {
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
  static void push(lua_State* L,
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

template <typename T>
struct pusher<std::set<T>, std::enable_if<true>::type> {
  static void push(lua_State* L, const std::set<T>& set) {
    lua_newtable(L);
    for (auto& i : set) {
      pusher<T>::push(L, i);
      lua_pushboolean(L, true);
      lua_settable(L, -3);
    }
  }
};

template <typename T>
struct pusher<std::unordered_set<T>, std::enable_if<true>::type> {
  static void push(lua_State* L, const std::unordered_set<T>& set) {
    lua_newtable(L);
    for (auto& i : set) {
      pusher<T>::push(L, i);
      lua_pushboolean(L, true);
      lua_settable(L, -3);
    }
  }
};

template <typename T>
void push_func(T func) {
  pusher<T>::push(func);
}
}
}
