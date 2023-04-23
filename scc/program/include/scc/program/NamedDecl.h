#pragma once

#include <string_view>

#include "scc/program/Decl.h"
#include "scc/program/IdentTable.h"

/// Any declaration that has a name.
class NamedDecl : public Decl {
  IdentTable::NameID nameId;

public:
  NamedDecl(Decl::Kind kind, IdentTable::NameID name)
      : Decl(kind), nameId(name) {}
  std::string_view getName(const IdentTable &idents) const;
  std::string_view getName(const Program &prog) const;
  IdentTable::NameID getNameID() const { return nameId; }
};
