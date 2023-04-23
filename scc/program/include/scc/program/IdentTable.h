#pragma once

#include "scc/program/Builtin.h"
#include "scc/utils/SCCAssert.h"
#include "scc/utils/StrongTypedef.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

/// Stores all the identifiers in a program.
struct IdentTable {
  /// This is a unique internal ID for an unqualified identifier (e.g. 'x').
  ///
  /// Note that there is no 1-1 relationship between identifiers and NameIDs.
  /// One NameID has exactly one string representation, but one string
  /// representation might have multiple NameIDs. For example, two local
  /// variables in two different scopes could be named 'x' but have different
  /// NameIDs.
  struct NameID : StrongTypedef<NameID> {};

  [[nodiscard]] const std::string &getName(NameID id) const {
    return names.at(id.getInternalVal()).name;
  }

  /// Create a new ID that has the given name.
  NameID createID(std::string name, bool fixed = false) {
    for (const auto &n : names)
      SCCAssert(n.name != name, "Duplicate identifier?");

    NameID result = NameID::fromInternalValue(names.size());
    names.emplace_back(name, fixed);
    return result;
  }

  NameID getOrCreateID(std::string name, bool fixed = false) {
    size_t index = 0;
    for (const NameInfo &n : names) {
      if (n.name == name)
        return NameID::fromInternalValue(index);
      ++index;
    }
    return createID(name, fixed);
  }

  bool hasID(const std::string name) const {
    for (const NameInfo &n : names) {
      if (n.name == name)
        return true;
    }
    return false;
  }

  bool isFixedID(NameID id) const {
    return names.at(id.getInternalVal()).fixed;
  }

  bool isValidID(NameID id) const {
    return id.getInternalVal() < getLastID().getInternalVal();
  }

  NameID getLastID() const { return NameID::fromInternalValue(names.size()); }

  bool isValidName(std::string s) const;

  /// Returns a new unique identifiers. Might have the given prefix
  /// (but this is not a promise).
  NameID makeNewID(std::string prefix = "i") {
    std::string name = prefix + std::to_string(names.size());
    return createID(name);
  }

  bool tryChangeId(NameID id, std::string newName) {
    SCCAssert(isValidID(id), "Trying to change invalid ID string?");
    SCCAssert(isValidName(newName),
              "Trying to change identifier to invalid str");
    if (names.at(id.getInternalVal()).fixed)
      return false;
    for (const auto &n : names)
      if (n.name == newName)
        return false;
    names.at(id.getInternalVal()).name = newName;
    return true;
  }

  void remove(NameID id) {
    names.at(id.getInternalVal()) = NameInfo();
    while (!names.empty() && !names.back().valid)
      names.pop_back();
  }

private:
  struct NameInfo {
    std::string name;
    /// True if the exact string representation matters. E.g., a fixed string
    /// would be 'int' (as that's a builtin type). A non-fixed string is a
    /// randomly generated variable name such as 'var123'.
    bool fixed = false;
    /// Whether this is a valid NameInfo object.
    bool valid = false;
    /// Constructs an invalid object.
    NameInfo() = default;

    NameInfo(std::string n, bool fixed) : name(n), fixed(fixed) {
      valid = true;
    }
  };

  /// List of stored names.
  ///
  /// The index is equal to the internal value of a NameID.
  std::vector<NameInfo> names;
};

typedef IdentTable::NameID NameID;
namespace std {
template <> struct hash<NameID> {
  std::size_t operator()(const NameID &k) const { return k.getInternalVal(); }
};
} // namespace std

/// An invalid NameID.
constexpr NameID InvalidName = std::numeric_limits<NameID>::max();
