#include "scc/program/RecordDecl.h"
#include "scc/program/Program.h"

Record::Record(Program &p, IdentTable::NameID name, bool packed)
    : NamedDecl(Decl::Kind::Record, name), packed(packed) {
  type = p.getTypes().addType(Type::Record(name));
}

void Record::printForwardDecl(PrintState &state) const {
  printCommonPrefix(state);
  state.getOut() << ";\n";
}

void Record::print(PrintState &state) const {
  const Program &p = state.getProgram();
  OutStream &out = state.getOut();
  printCommonPrefix(state);
  out << " {\n";
  for (const Field &f : fields) {
    out << "  ";
    p.getTypes().get(f.getType()).print(state);
    out << " " << p.getIdents().getName(f.getName());
    if (f.isBitfield())
      out << " : " << *f.getBitfieldSize();

    if (f.getMinAlignment() != 1) {
      state.getOut() << " __attribute__((aligned(" << f.getMinAlignment()
                     << ")))";
    }
    out << ";\n";
  }
  out << "};\n";
}

bool Record::usesType(TypeRef t) const {
  // FIXME: This returns true for records containing their own type as a
  // field.
  for (const Field &f : fields)
    if (f.getType() == t)
      return true;
  return false;
}

bool Record::referencesID(NameID id) const {
  for (const Field &f : fields)
    if (f.getName() == id)
      return true;
  return getNameID() == id;
}

SourceDependencies Record::getDependencies(const Program &p) const {
  SourceDependencies result;
  for (const auto &field : getFields())
    result.add(SourceDependency(field.getType()));
  return result;
}

void Record::printCommonPrefix(PrintState &state) const {
  state.getOut() << (isUnion() ? "union " : "struct ");
  state.getOut() << state.getProgram().getIdents().getName(getNameID());
}
