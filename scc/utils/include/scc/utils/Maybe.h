#ifndef MAYBE_H
#define MAYBE_H

#include "scc/utils/Error.h"
#include "scc/utils/SCCAssert.h"

#include <string>
#include <variant>

/// Either a given value or an error.
template <typename T> class Maybe {
  std::variant<T, Error> storage;

  std::string SCCAssertInfo() const {
    return "Error:\n" + takeError().getMessage();
  }

public:
  Maybe() { storage = Error::withMsg("Unknown default error"); }
  Maybe(T v) : storage(v) {}

  Maybe(Error w) : storage(w) {}

  explicit operator bool() const { return !isErr(); }

  [[nodiscard]] Error takeError() const {
    SCCAssert(isErr(), "Trying to take error of Maybe without error");
    return std::get<Error>(storage);
  }

  [[nodiscard]] std::string getErrorMsg() const {
    return takeError().getMessage();
  }

  [[nodiscard]] bool isErr() const {
    return std::get_if<Error>(&storage) != nullptr;
  }

  const T &assumeValid(const char *msg) const {
    SCCAssert(!isErr(), "Assumption failed: " + std::string(msg));
    return **this;
  }

  const T &operator*() const {
    SCCAssert(!isErr(), "Trying to deref Maybe in error state.");
    return std::get<T>(storage);
  }

  T &operator*() {
    SCCAssert(!isErr(), "Trying to deref Maybe in error state.");
    return std::get<T>(storage);
  }

  const T *operator->() const { return &**this; }
  T *operator->() { return &**this; }
};

#endif // MAYBE_H
