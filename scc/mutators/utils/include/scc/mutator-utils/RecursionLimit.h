#ifndef RECURSIONLIMIT_H
#define RECURSIONLIMIT_H

#include <cassert>

/// Utility class for enforcing a recursion limit in a function.
struct RecursionLimit {
  unsigned limit = 1;
  unsigned currentDepth = 0;
  /// Creates a recursion limit.
  RecursionLimit(unsigned limit) : limit(limit) { assert(limit > 0); }

  /// RAII class that is used to mark that we're in a (recursive) call
  /// at the moment.
  struct Scope {
    RecursionLimit *limit = nullptr;
    Scope(RecursionLimit &l) : limit(&l) { limit->currentDepth += 1; }
    ~Scope() { limit->currentDepth -= 1; }
    /// True if we currently reached or exceeded the recursion limit.
    bool reached() const { return limit->currentDepth >= limit->limit; }
  };

  const Scope scope() { return Scope(*this); }
};

#endif // RECURSIONLIMIT_H
