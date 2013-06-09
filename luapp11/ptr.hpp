#pragma once

namespace luapp11 {

template <typename T> class ptr {
 public:
  ~ptr();

  T* get() const {
    return ptr_;
  }

  T& operator*() const {
    return *ptr_;
  }

  T& operator->() const {
    return *ptr_;
  }

  explicit operator bool() const {
    return ptr_ == nullptr;
  }
 private:
  ptr(T*);

  T* ptr_;
};

}