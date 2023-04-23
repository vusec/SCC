#pragma once

#include "scc/utils/StrongTypedef.h"
#include <cstddef>

struct TypeRef : StrongTypedef<TypeRef> {};
namespace std {
template <> struct hash<TypeRef> {
  std::size_t operator()(const TypeRef &k) const { return k.getInternalVal(); }
};
} // namespace std

namespace debugSupport {
inline std::string toString(TypeRef value) {
  return "Type: " + std::to_string(value.getInternalVal());
}
} // namespace debugSupport

/// Void is special and always the first type, so make available everywhere.
constexpr TypeRef Void() { return TypeRef::fromInternalValue(0U); }

using ByteSize = unsigned;
enum class LoopCtrl {
  Continue,
  Abort,
};
