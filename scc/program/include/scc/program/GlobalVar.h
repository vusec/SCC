#ifndef GLOBALVAR_H
#define GLOBALVAR_H

#include "scc/program/NamedDecl.h"
#include "scc/program/Statement.h"
#include "scc/program/Variable.h"

/// Represents a global variable in the target program.
class GlobalVar : public NamedDecl {
  TypeRef type = Void();
  Statement initializer;

public:
  GlobalVar(TypeRef t, NameID id)
      : NamedDecl(Decl::Kind::GlobalVar, id), type(t) {
    assert(t != Void());
  }

  void setInit(Statement s) { initializer = s; }

  bool is_static = true;

  Variable getAsVar() const { return Variable(type, getNameID()); }

  Decl *clone() const override { return new GlobalVar(*this); }

  void printForwardDecl(PrintState &state) const override;
  void print(PrintState &state) const override;

  bool usesType(TypeRef t) const override {
    return initializer.usesType(t) || this->type == t;
  }

  bool referencesID(NameID id) const override { return false; }

  size_t countNodes() const override { return 2 + initializer.countNodes(); }

  SourceDependencies getDependencies(const Program &p) const override;

  void verifySelf(const Program &p) const override {}
};

#endif // GLOBALVAR_H
