#pragma once

#include "luapp11/val.hpp"
#include "luapp11/internal/stack_guard.hpp"
#include "luapp11/internal/is.hpp"
#include "luapp11/internal/caller.hpp"

namespace luapp11 {
namespace internal {
struct iterator_impl;
}
class stack_var {
 public:
  stack_var() = delete;
  stack_var(const stack_var& other) = delete;
  stack_var(stack_var&& other);
  stack_var& operator=(stack_var&& other);

  /**
   * Gets the value from this place in the stack.
   * @return The value found there.
   */
  val get_value() const { return val(L, index_); }

  /**
   * Gets the value from this place in the stack.
   * @typename T The type to get.
   * @return     The value found there.
   */
  template <typename T>
  T get() const {
    return get_value().get<T>();
  }

  /**
    * Checks if the value at this place in the stack can be converted
    * to the specified type.
    * @typename T The type to check.
    * @return     true if the value can be converted false otherwise.
    */
  template <typename T>
  bool is() const {
    return internal::typed_is<T>::is(L, index_);
  }

  bool is_table() const {
    return !lua_isnoneornil(L, index_) && lua_istable(L, index_);
  }

  /**
   * Gets the value from this place in the stack.  Returns the default
   * value if unable to convert to the requested type.
   * @typename T        The type to get.
   * @param    fallback The value to return if type convertion fails.
   * @return            The value found there, or the default value if
   * conversion fails.
   */
  template <typename T>
  T as(T&& fallback) const {
    return is<T>() ? get<T>() : fallback;
  }

  /**
   * Assigns the value at one place in thet lua environment to another.
   * @stack_var    The location to assign from.
   */
  void assign(val key, const stack_var& var) {
    key.push(L);
    if (L == var.L) {
      internal::stack_guard g2(L, true);
      var.push();
    } else {
      var.get_value().push(L);
    }
    lua_settable(L, index_);
  }

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  void assign(val key, const T& toSet) {
    key.push(L);
    val(toSet).push(L);
    lua_settable(L, index_);
  }

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  void assign(val key, const std::initializer_list<T>& toSet) {
    key.push(L);
    val::pusher<std::initializer_list<T>>::push(L, toSet);
    lua_settable(L, index_);
  }

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  void assign(val key, const std::initializer_list<val>& toSet) {
    key.push(L);
    val::pusher<std::initializer_list<val>>::push(L, toSet);
    lua_settable(L, index_);
  }

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  void assign(val key,
              const std::initializer_list<std::pair<val, val>>& toSet) {
    key.push(L);
    val::pusher<std::initializer_list<std::pair<val, val>>>::push(L, toSet);
    lua_settable(L, index_);
  }

  bool operator==(const stack_var& other) const {
    return std::tie(L, index_) == std::tie(L, index_);
  }

  template <typename T>
  bool operator==(const T& other) const {
    return is<T>() && get<T>() == other;
  }

  template <typename T>
  bool operator!=(const T& other) const {
    return !this->operator==(other);
  }

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The index to get.
   * @return      A stack_var which points to the child of this location at
   * index
   * idx.
   */
  stack_var operator[](val idx) const {
    internal::stack_guard g(L);
    idx.push(L);
    lua_gettable(L, index_);
    return stack_var(L, lua_gettop(L), std::move(g));
  }

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The location of the index to get.
   * @return      A stack_var which points to the child of this location at
   * index
   * idx.
   */
  stack_var operator[](stack_var idx) const;

  /**
   * Attempt to call the function at this location in the lua environment.
   * @param args  The arguments to the call
   */
  template <typename... TArgs>
  result<void> operator()(TArgs... args) const {
    return invoke<void>(args...);
  }

  /**
   * Attempt to call the function at this location in the lua environment.  Can
   * return a value.
   * @param args  The arguments to the call
   * @return      The result of the invocation.
   */
  template <typename TOut, typename... TArgs>
  result<TOut> invoke(TArgs... args) const {
    if (is<TOut(TArgs...)>()) {
      val::push_all<TArgs...>(L, args...);
      return caller<TOut>::pcall(L, sizeof...(TArgs));
    }
    throw exception("Tried to invoke non-function.", L);
  }

  template <typename TOut>
  result<TOut> invoke() const {
    if (is<TOut()>()) {
      return caller<TOut>::pcall(L, 0);
    }
    throw exception("Tried to invoke non-function.", L);
  }

  template <typename T>
  struct iterator_impl {
   public:
    iterator_impl(lua_State* L, int idx)
        : L(L),
          idx(idx),
          pair(T(L, lua_gettop(L) + 1, internal::stack_guard(L)),
               T(L, lua_gettop(L) + 2, internal::stack_guard(L))) {
      lua_pushnil(L);
      inc();
    }

    bool operator==(const iterator_impl& other) {
      return std::tie(L, idx, pair) == std::tie(other.L, other.idx, other.pair);
    }

    void inc() {
      if (!lua_next(L, idx)) {
        L = nullptr;
      }
    }

    lua_State* L;
    int idx;
    std::pair<T, T> pair;
  };

  class iterator : public std::iterator<std::forward_iterator_tag,
                                        std::pair<stack_var, stack_var>> {
   public:
    iterator() {}
    ~iterator() {}
    iterator(iterator&& other) { swap(*this, other); }

    iterator& operator++() {
      impl_->inc();
      if (!impl_->L) {
        impl_.reset();
      }
      return *this;
    }

    bool operator==(const iterator& other) {
      return !impl_ ? !other.impl_ : *impl_ == *other.impl_;
    }
    bool operator!=(const iterator& other) {
      return !(this->operator==(other));
    }
    iterator& operator=(iterator&& other) {
      swap(*this, other);
      return *this;
    }
    std::pair<stack_var, stack_var>& operator*() { return impl_->pair; }
    std::pair<stack_var, stack_var>* operator->() { return &impl_->pair; }
    friend void swap(iterator&& a, iterator&& b)  // nothrow
    {
      using std::swap;

      swap(a.impl_, b.impl_);
    }

   private:
    iterator(lua_State* L, int idx)
        : impl_(new iterator_impl<stack_var>(L, idx)) {}

    std::unique_ptr<iterator_impl<stack_var>> impl_;
    friend class stack_var;
  };

  iterator begin() { return iterator(L, index_); }
  iterator end() { return iterator(); }
  iterator begin() const { return iterator(L, index_); }
  iterator end() const { return iterator(); }
  // const_iterator cbegin() const;
  // const_iterator cend() const;

 private:
  // Invoking
  template <typename T, class Enable = void>
  struct caller {
    static result<T> call(lua_State* L, int nargs) {
      lua_call(L, nargs, 1);
      return val::popper<T>::get(L, -1);
    }
    static result<T> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, 1, 0);
      if (err) {
        return error(err, "Error calling lua method.", L);
      }
      return val::popper<T>::get(L, -1);
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
        return error(err, "Error calling lua method", L);
      }
      return result<T>();
    }
  };

  template <typename... TArgs>
  struct caller<std::tuple<TArgs...>, std::enable_if<true>::type> {
    static result<std::tuple<TArgs...>> call(lua_State* L, int nargs) {
      lua_call(L, nargs, sizeof...(TArgs));
      return val::popper<std::tuple<TArgs...>>::get(L,
                                                    (int)sizeof...(TArgs) * -1);
    }
    static result<std::tuple<TArgs...>> pcall(lua_State* L, int nargs) {
      auto err = lua_pcall(L, nargs, sizeof...(TArgs), 0);
      if (err) {
        return error(err, "Error calling lua method.", L);
      }
      return val::popper<std::tuple<TArgs...>>::get(L,
                                                    (int)sizeof...(TArgs) * -1);
    }
  };

  void push() const { lua_pushvalue(L, index_); }

  // Private Constructors
  stack_var(lua_State* L, int index, internal::stack_guard&& g)
      : L{L}, index_{index}, g_(std::forward<internal::stack_guard>(g)) {}
  lua_State* L;
  int index_;
  internal::stack_guard g_;

  friend class var;
};
}

namespace std {
inline string to_string(const luapp11::stack_var& v, string indent = "") {
  if (!v.is_table()) {
    return v.as<string>("Nil");
  }
  string str = "{\n";
  for (auto& pair : v) {
    str += indent + pair.first.get<string>() + ":" +
           to_string(pair.second, indent + "  ") + "\n";
  }
  str += "}\n";
  return str;
}
}
