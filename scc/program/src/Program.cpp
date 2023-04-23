#include "scc/program/Program.h"

#include <iostream>

#include "scc/program/RecordDecl.h"

Program::Program() : builtin(*this) {}

OptError Program::print(OutStream &s) const {
  PrintState state(*this, s);

  OptError err = global.print(state);

  s << "\n";
  return err;
}

namespace {
struct SinkOutStream : OutStream {
  virtual void writeImpl(std::string_view s) {}
};
} // namespace

bool Program::canPrint() const {
  SinkOutStream sink;
  return print(sink).isSuccess();
}

bool Program::isTypeUsed(TypeRef t) const {
  for (const Type &other : types)
    if (other.usesType(t))
      return true;
  for (const Decl *d : getDeclList())
    if (d->usesType(t))
      return true;
  return false;
}

bool Program::isIDUsed(NameID id) const {
  for (const Decl *d : getDeclList())
    if (d->referencesID(id))
      return true;
  return false;
}

void Program::dump(bool fancy) const {
  OutString str(fancy);
  print(str).assumeSuccess("Failed to dump program");
  std::cerr << "\n\n\nPROGRAM:\n" << str.getStr() << "\n";
}

std::string Program::toDebugStr() const {
  OutString str(/*fancy=*/true);
  print(str).assumeSuccess("Failed to dump program");
  return str.getStr();
}

size_t Program::countNodes() const {
  size_t count = 0;

  for (const Decl *d : getDeclList()) {
    count += 1;
    count += d->countNodes();
  }

  count += types.countNodes();

  return count;
}

Function *Program::getFunctionWithID(NameID id) {
  for (Decl *d : getDeclList()) {
    if (d->getKind() != Decl::Kind::Function)
      continue;
    Function *f = static_cast<Function *>(d);
    if (f->getNameID() == id)
      return f;
  }
  return nullptr;
}

const Record &Program::getRecord(TypeRef t) const {
  SCCAssert(types.get(t).isRecord(), "Must be a record");
  for (Decl *d : getDeclList()) {
    if (d->getKind() != Decl::Kind::Record)
      continue;
    const Record &record = *static_cast<Record *>(d);
    if (record.getNameID() != types.get(t).getRecordNameID())
      continue;
    return record;
  }
  SCCError("Couldn't find record?");
}

const Decl &Program::lookup(NameID id) const {
  Decl *result = global.find(id);
  SCCAssert(result, "Failed to lookup decl?");
  return *result;
}
