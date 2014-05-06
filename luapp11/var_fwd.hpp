#pragma once

#include "luapp11/fwd.hpp"
#include "luapp11/internal/iterator_fwd.hpp"

namespace luapp11 {

class var {
 public:
  var(const var& other) = default;
  var(var&& other) = default;

  /**
   * Gets the value from this place in the lua environment.
   * @return The value found there.
   */
  val get_value() const;
  /**
   * Gets the value from this place in the lua environment.
   * @typename T The type to get.
   * @return     The value found there.
   */
  template <typename T>
  T get() const;

  /**
    * Checks if the value at this place in the lua environment can be converted
   * to the specified type.
    * @typename T The type to check.
    * @return     true if the value can be converted false otherwise.
    */
  template <typename T>
  bool is() const;

  bool is_table() const;

  /**
   * Gets the value from this place in the lua environment.  Returns the default
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
   * @var    The location to assign from.
   * @return The location assigned to.
   */
  var& operator=(const var& var);

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  var& operator=(const T& toSet);

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  template <typename T>
  var& operator=(const std::initializer_list<T>& toSet);

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<val>& toSet);

  /**
   * Assigns a value to this place in the lua environment.
   * @param  toSet  The value to assign.
   * @return        The location assigned to.
   */
  var& operator=(const std::initializer_list<std::pair<val, val>>& toSet);

  /**
   * Checks if two vars point to the same place in the lua environment.
   */
  bool operator==(const var& other) const;

  template <typename T>
  bool operator==(const T& other) const;

  bool operator!=(const var& other) const;

  template <typename T>
  bool operator!=(const T& other) const;

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The index to get.
   * @return      A var which points to the child of this location at index idx.
   */
  var operator[](val idx) const;

  /**
   * Get a child of this location in the lua environment.
   * @param  idx  The location of the index to get.
   * @return      A var which points to the child of this location at index idx.
   */
  var operator[](var idx) const;

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

  /**
   * Execute a string as lua.  Assigns it's return value to this location in the
   * lua environment.
   * @param  str The lua code to execute.
   * @return     The error (if any) which occured while executing.
   */
  error do_chunk(const std::string& str);

  /**
   * Execute a lua file.  Assigns it's return value to this location in the lua
   * environment.
   * @param  path The location of the file on disk.
   * @return      The error (if any) which occured while executing.
   */
  error do_file(const std::string& path);

  template <typename T, typename... TArgs>
  ptr<T> create(TArgs... args);

  typedef internal::iterator<var> iterator;
  typedef internal::iterator<const var> const_iterator;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

 private:
  // Pushing
  void push_parent_key() const;

  void push() const;

  template <typename T>
  bool dirty_is() const;

  // Metastuff
  template <typename T>
  void setup_metatable() const;

  // Private Constructors
  var(lua_State* L, int virtual_index, val key);
  var(var v, val key);

  lua_State* L;
  std::vector<val> lineage_;
  int virtual_index_;

  friend class internal::core_access;
};
}