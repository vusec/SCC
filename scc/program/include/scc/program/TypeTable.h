#pragma once

#include "scc/program/Builtin.h"
#include "scc/program/Type.h"

#include <optional>
#include <vector>

/// Contains all types in a program.
class TypeTable {
  /// List of types. Indices are values of TypeRef
  /// values.
  std::vector<Type> types;

public:
  /// Add a new type to the list of types.
  TypeRef addType(Type t);

  std::optional<TypeRef> getTypeForRecord(NameID record);

  /// Given a TypeRef, return the associated Type.
  Type &get(TypeRef t) {
    SCCAssert(t.getInternalVal() < types.size(), "Invalid TypeRef?");
    return types.at(t.getInternalVal());
  }

  const Type &getUnqualified(TypeRef t) const {
    TypeRef unqualified = stripCV(t);
    return get(unqualified);
  }

  const Type &get(TypeRef t) const {
    SCCAssert(t.getInternalVal() < types.size(), "Invalid TypeRef?");
    return types.at(t.getInternalVal());
  }

  void eraseType(TypeRef t) {
    get(t) = Type();
    for (Type &other : types) {
      if (other.usesType(t))
        eraseType(other.getRef());
    }
    shrinkToFit();
  }

  bool isValid(TypeRef t) const {
    return get(t).getKind() != Type::Kind::Invalid;
  }

  bool isConst(TypeRef t) { return (get(t).getKind() == Type::Kind::Const); }

  bool isVolatile(TypeRef t) {
    return (get(t).getKind() == Type::Kind::Volatile);
  }

  TypeRef stripCV(TypeRef t) const {
    const Type &resolved = get(t);
    if (resolved.isCVQualified())
      return stripCV(resolved.getBase());
    return t;
  }

  /// Returns the TypeRef for a specific derived type (or nothing
  /// in case there is no derived type).
  std::optional<TypeRef> hasDerivedType(TypeRef base, Type::Kind d) {
    for (const Type &t : types)
      if (t.getKind() == d && t.getBase() == base)
        return t.getRef();
    return {};
  }

  std::optional<TypeRef> hasArrayType(TypeRef base, unsigned size) {
    for (const Type &t : types)
      if (t.getKind() == Type::Kind::Array && t.getBase() == base &&
          t.getArraySize() == size)
        return t.getRef();
    return {};
  }

  TypeRef getOrCreateDerived(IdentTable &idents, Type::Kind d, TypeRef base) {
    if (std::optional<TypeRef> found = hasDerivedType(base, d))
      return *found;
    return addType(Type(idents, *this, d, base));
  }

  TypeRef getOrCreateArray(IdentTable &idents, TypeRef base, unsigned size) {
    if (std::optional<TypeRef> found = hasArrayType(base, size))
      return *found;

    Type t = Type::Array(base, size, idents.makeNewID("a"));
    return addType(t);
  }

  auto begin() { return types.begin(); }
  auto end() { return types.end(); }
  auto begin() const { return types.begin(); }
  auto end() const { return types.end(); }

  size_t countNodes() const {
    size_t res = 0;
    for (const Type &t : types)
      if (t.getKind() != Type::Kind::Invalid)
        res += 1;
    return res;
  }

  void shrinkToFit() {
    while (!types.empty() && types.back().getKind() == Type::Kind::Invalid)
      types.pop_back();
  }
};
