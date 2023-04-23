#ifndef ERROR_H
#define ERROR_H

#include "scc/utils/SCCAssert.h"

#include <string>

/// Utility class for representing an error.
struct Error {
  static Error withMsg(std::string msg) {
    Error e;
    e.valid = true;
    e.msg = msg;
    return e;
  }

  explicit Error() = default;

  std::string getMessage() const {
    SCCAssert(valid, "called on invalid error?");
    return msg;
  }

private:
  bool valid = false;
  std::string msg;
};

[[nodiscard]] static inline Error Err(std::string error) {
  return Error::withMsg(error);
}

struct OptError {
  OptError(Error e) : error(e) {}
  OptError() = default;

  void assumeSuccess(std::string msg) {
    if (error) {
      const std::string fullMessage = msg + ". Reason: " + error->getMessage();
      SCCError(fullMessage);
    }
  }

  bool hasError() const { return static_cast<bool>(error); }

  bool isSuccess() const { return !hasError(); }

  operator bool() const { return hasError(); }

  const Error &operator*() const {
    SCCAssert(error.has_value(), "Trying to access no error?");
    return *error;
  }

  const Error *operator->() const {
    SCCAssert(error.has_value(), "Trying to access no error?");
    return &*error;
  }

private:
  std::optional<Error> error;
};

#endif // ERROR_H
