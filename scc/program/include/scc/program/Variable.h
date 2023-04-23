#pragma once

#include "scc/program/TypeTable.h"

#include <optional>

/// Represents the necessary information for describing a variable.
class Variable {
  TypeRef t = Void();
  NameID name;

public:
  Variable() = default;
  Variable(TypeRef t, NameID name) : t(t), name(name) {}
  TypeRef getType() const { return t; }
  NameID getName() const { return name; }
  bool operator==(const Variable &o) const {
    return std::tie(t, name) == std::tie(o.t, o.name);
  }
  bool operator!=(const Variable &o) const { return !(*this == o); }
  bool operator<(const Variable &o) const {
    return std::tie(t, name) < std::tie(o.t, o.name);
  }
};

typedef std::optional<Variable> OptVar;
