#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include "scc/program/Builtin.h"
#include "scc/utils/Maybe.h"
#include "scc/utils/SCCAssert.h"

#include <vector>

class Decl;
class Program;
class Statement;

/// A single source dependency.
struct SourceDependency {
  /// Whether this dependency points to a type or a decl.
  enum class Kind { Type, Decl };

  /// @see Kind
  Kind getKind() const { return kind; }

  /// The type this dependency points to if this is a type dependency.
  TypeRef getType() const {
    SCCAssert(kind == Kind::Type, "Can only be called on type dependencies");
    return type;
  }

  /// The decl this dependency points to if this is a decl dependency.
  const Decl &getDecl() const {
    SCCAssert(kind == Kind::Decl, "Can only be called on decl dependencies");
    return *decl;
  }

  SourceDependency() = default;
  explicit SourceDependency(TypeRef t) : type(t), kind(Kind::Type) {}
  explicit SourceDependency(const Decl &decl) : decl(&decl), kind(Kind::Decl) {}

  bool operator==(const SourceDependency &d) const {
    return std::tie(type, decl, kind) == std::tie(d.type, d.decl, d.kind);
  }

private:
  TypeRef type = Void();
  const Decl *decl = nullptr;
  Kind kind = Kind::Type;
};

namespace std {
template <> struct hash<SourceDependency> {
  size_t operator()(const SourceDependency &x) const {
    switch (x.getKind()) {
    case SourceDependency::Kind::Decl:
      return std::hash<const Decl *>()(&x.getDecl());
    case SourceDependency::Kind::Type:
      return std::hash<TypeRef::Underlying>()(x.getType().getInternalVal());
    }
    SCCError("Missing switch?");
  }
};
} // namespace std

/// Represents dependencies between declarations/types.
///
/// E.g., the variable 'struct Foo i;' has a dependency on the type 'Foo'
/// because it needs to be defined before the variable is defines.
struct SourceDependencies {
  /// Creates an empty dependency set.
  SourceDependencies() = default;

  /// Add the given dependency.
  void add(SourceDependency d) { values.push_back(d); }

  /// Add all dependencies to this list of dependencies.
  void merge(const SourceDependencies &other) {
    for (const auto &dependency : other.values)
      add(dependency);
  }

  /// Returns all dependencies in the order they should be printed in the
  /// source code.
  Maybe<std::vector<SourceDependency>> getOrdered(const Program &p);

  // Iterator support.
  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }

  /// Returns the dependencies of the given statement.
  [[nodiscard]] static SourceDependencies fromStmt(const Program &p,
                                                   const Statement &s);

  bool operator==(const SourceDependencies &other) const {
    return values == other.values;
  }

private:
  std::vector<SourceDependency> values;

  /// Returns the direct dependencies of the given part of the program.
  SourceDependencies getDirectDependencies(SourceDependency dep,
                                           const Program &p) const;
};

#endif // DEPENDENCIES_H
