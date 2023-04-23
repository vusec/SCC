#include "scc/program/Decl.h"

// pin vtable
Decl::~Decl() {}

// pin vtable
DeclExtraData::~DeclExtraData() {}

void Decl::dump(const Program &p) const {
  StdErrOutStream s;
  PrintState state(p, s);
  print(state);
}
