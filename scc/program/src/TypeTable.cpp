#include "scc/program/TypeTable.h"

TypeRef TypeTable::addType(Type t) {
  // Reuse an existing invalid type first.
  size_t index = 0;
  for (Type &existing : types) {
    if (existing.getKind() == Type::Kind::Invalid) {
      // Link the reference.
      TypeRef res = TypeRef::fromInternalValue(index);
      t.ref = res;

      // Overwrite the type.
      existing = t;
      return res;
    }
    ++index;
  }

  TypeRef res = TypeRef::fromInternalValue(types.size());
  t.ref = res;
  types.push_back(t);
  return res;
}

std::optional<TypeRef> TypeTable::getTypeForRecord(NameID record) {
  for (const Type &t : types) {
    if (t.isRecord() && t.getRecordNameID() == record)
      return t.getRef();
  }
  return {};
}
