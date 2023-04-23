#include "scc/program/NamedDecl.h"
#include "scc/program/Program.h"

std::string_view NamedDecl::getName(const IdentTable &idents) const {
  return idents.getName(nameId);
}

std::string_view NamedDecl::getName(const Program &prog) const {
  return getName(prog.getIdents());
}
