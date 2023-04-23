#ifndef LANGOPTS_H
#define LANGOPTS_H

#include "scc/utils/Error.h"

#include <optional>
#include <string>

/// Describes a programming language (e.g., C++ or C89).
class LangOpts {
public:
  /// The language standard used.
  enum class Standard {
    C89,
    C99,
    Cxx03,
    Cxx11,
    Cxx17,
  };

private:
#define LANG_OPT(type, id, str, def) type id = def;
#include "LangOpts.def"

  Standard standardEnum = Standard::C99;

public:
  bool isCxx() const {
    switch (standardEnum) {
    case Standard::Cxx03:
    case Standard::Cxx11:
    case Standard::Cxx17:
      return true;
    default:
      return false;
    }
    return false;
  }

  void setStandard(Standard s) { standardEnum = s; }

  bool isC() const { return !isCxx(); }

  bool hasUInt128() const { return HasUInt128; }

  OptError loadFromFile(std::string path);
};

#endif // LANGOPTS_H
