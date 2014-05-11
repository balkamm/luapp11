#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/iterator_fwd.hpp"

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
  val get_value() const;

  /**
   * Gets the value from this place in the stack.
   * @typename T The type to get.
   * @return     The value found there.
   */
  template <typename T>
  T get() const;

  /**
    * Checks if the value at this place in the stack can be converted
    * to the specified type.
    * @typename T The type to check.
    * @return     true if the value can be converted false otherwise.
    */
  template <typename T>
  bool is() const;

  bool is_table() const;

  /**
   * Gets the value from this place in the stack.  Returns the default
   * value if unable to convert to the requested type.
   * @typename T        The type to get.
   * @param    fallback The value to return if type convertion fails.
   * @return            The value found there, or the default value if
   * conversion fails.
   */
  template <typename T>
  T as(T&& fallback) const;

  /**
   * Assigns the value at one place in thet lua environment to another.
   * @stack_var    The location to assign from.
   */
  void assign(val key, const stack_var& stack_var);

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  void assign(val key, const T& toSet);

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  void assign(val key, const std::initializer_list<T>& toSet);

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  void assign(val key, const std::initializer_list<val>& toSet);

  /**
   * Assigns a value to this place in the stack.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  void assign(val key, const std::initializer_list<std::pair<val, val>>& toSet);

  template <typename T>
  bool operator==(const T& other) const;

  template <typename T>
  bool operator!=(const T& other) const;

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The index to get.
   * @return      A stack_var which points to the child of this location at
   * index
   * idx.
   */
  stack_var operator[](val idx) const;

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
  result<void> operator()(TArgs... args) const;

  /**
   * Attempt to call the function at this location in the lua environment.  Can
   * return a value.
   * @param args  The arguments to the call
   * @return      The result of the invocation.
   */
  template <typename TOut, typename... TArgs>
  result<TOut> invoke(TArgs... args) const;

  template <typename TOut>
  result<TOut> invoke() const;

  class iterator : public std::iterator<std::forward_iterator_tag,
                                        std::pair<stack_var, stack_var>> {
   public:
    iterator();
    iterator(const iterator&) = delete;
    iterator(iterator&& other);
    iterator& operator++();
    bool operator==(const iterator& other);
    bool operator!=(const iterator& other);
    iterator& operator=(iterator&& other);
    std::pair<stack_var, stack_var>& operator*();
    std::pair<stack_var, stack_var>* operator->();

    friend void swap(iterator&& a, iterator&& b);

   private:
    iterator(lua_State* L, int idx);

    std::unique_ptr<internal::iterator_impl> impl_;
  };

  iterator begin();
  iterator end();
  iterator begin() const;
  iterator end() const;
  // const_iterator cbegin() const;
  // const_iterator cend() const;

 private:
  // Private Constructors
  stack_var(lua_State* L, int index, internal::stack_guard&& g);

  lua_State* L;
  int index_;
  internal::stack_guard g_;

  friend class internal::core_access;
};
}

namespace std {
string to_string(const luapp11::stack_var& v, string indent = "") {
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