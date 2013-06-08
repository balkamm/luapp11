#pragma once

namespace luapp11 {
namespace detail {
template <typename T, class Enable = void>
struct remove_function_ptr_member_type {
  typedef T type;
};

template <typename TOut, typename T, typename ... TArgs>
struct remove_function_ptr_member_type<TOut(T::*)(TArgs ...),
                                       std::enable_if<true>::type> {
  typedef TOut(type)(TArgs ...);
};

template <typename TOut, typename T, typename ... TArgs>
struct remove_function_ptr_member_type<TOut(T::*)(TArgs ...) const,
                                       std::enable_if<true>::type> {
  typedef TOut(type)(TArgs ...);
};

template <typename T, class Enable = void>
struct convert_functor_to_std_function {
  typedef T type;
};

template <typename T>
struct convert_functor_to_std_function<
    T,
    typename std::enable_if<std::is_member_function_pointer<
        decltype(&T::operator())>::value>::type> {
  typedef std::function<typename remove_function_ptr_member_type<
      decltype(&T::operator())>::type> type;
};
}
}