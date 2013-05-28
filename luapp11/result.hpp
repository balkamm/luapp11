#pragma once

namespace luapp11 {

template <typename T> class result {
 public:
  ~result() {}
  result(const result& r) : success_ { r.success_ }
  {
    if (success_) {
      val_ = r.val_;
    } else {
      err_ = r.err_;
    }
  }

  bool success() const { return success_; }

  error error() const {
    if (success_) {
      throw exception("Trying to get error from successful result.");
    }
    return err_;
  }

  T value() const {
    if (!success_) {
      throw exception("Trying to get value from failed result.");
    }
    return val_;
  }

  const operator T() const { return value(); }

  const explicit operator bool() const { return success_; }

 private:
  result(luapp11::error && err) : err_ { err }
  , success_ { false }
  {}
  result(const luapp11::error& err) : err_ { err }
  , success_ { false }
  {}
  result(T && val) : val_ { val }
  , success_ { true }
  {}
  result(const T& val) : val_ { val }
  , success_ { true }
  {}

  bool success_;
  union {
    luapp11::error err_;
    T val_;
  };
  friend class var;
};

template <> class result<void> {
 public:

  bool has_value() const { return success_; }

  error error() const { return err_; }

  const operator bool() const { return success_; }

 private:
  result() : success_ { true }
  {}
  result(luapp11::error && err) : err_ { err }
  , success_ { false }
  {}
  result(const luapp11::error& err) : err_ { err }
  , success_ { false }
  {}

  bool success_;
  luapp11::error err_;
  friend class var;
};

}