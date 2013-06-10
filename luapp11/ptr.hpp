#pragma once

namespace luapp11 {

template <typename T> class ptr {
 public:
  ~ptr() = default;

  T* get() const { return ptr_; }
  T& operator*() const { return *ptr_; }
  T* operator->() const { return ptr_; }

  bool operator==(const ptr<T>& other) const {
  	return ptr_ == other.ptr_;
  }

  explicit operator bool() const { return ptr_ == nullptr; }
 private:
  ptr(T* p) : ptr_ { p }
  {}

  T* ptr_;
  friend class val;
};

}