#include "scc/program/Type.h"
#include "scc/program/Program.h"
#include "scc/program/RecordDecl.h"

Type::Type(IdentTable &idents, const TypeTable &types, Kind d, TypeRef base) {
  kind = d;
  switch (d) {
  case Kind::Pointer:
    // FIXME: Not true for all systems.
    size = 8;
    this->base = base;
    break;
  case Kind::Volatile:
  case Kind::Const:
    this->base = base;
    break;
  case Kind::FunctionPointer:
  case Kind::Record:
  case Kind::Array:
  case Kind::Invalid:
  case Kind::Basic:
    SCCError("Can't create derived type");
    break;
  }
}

const Type &Type::strip(const TypeTable &t, std::vector<Kind> toStrip) const {
  const Type *ptr = this;
  while (true) {
    bool shouldStrip = std::find(toStrip.begin(), toStrip.end(),
                                 ptr->getKind()) != toStrip.end();
    if (!shouldStrip)
      return *ptr;
    ptr = &t.get(ptr->getBase());
  }
}

void Type::print(PrintState &state) const {
  const Program &prog = state.getProgram();
  OutStream &out = state.getOut();
  switch (getKind()) {
  case Kind::Pointer:
    prog.getTypes().get(base).print(state);
    out << "*";
    return;
  case Kind::FunctionPointer:
    out << prog.getIdents().getName(id);
    return;
  case Kind::Record:
    out << "struct ";
    out << prog.getRecord(getRef()).getName(prog);
    out << " ";
    return;
  case Kind::Array:
    out << prog.getIdents().getName(id);
    return;
  case Kind::Basic:
    out.printColor(OutStream::Color::Green, prog.getIdents().getName(id));
    return;
  case Kind::Const:
    prog.getTypes().get(base).print(state);
    out.printColor(OutStream::Color::Green, " const ");
    return;
  case Kind::Volatile:
    prog.getTypes().get(base).print(state);
    out.printColor(OutStream::Color::Green, " volatile ");
    return;
  case Kind::Invalid:
    out << "/*INVALID TYPE*/";
    return;
  }
  SCCError("Unimplemented type printing?");
}

typedef int (*ptr)(int *);

void Type::printPreamble(PrintState &state) const {
  const Program &prog = state.getProgram();
  OutStream &out = state.getOut();

  if (getKind() == Kind::FunctionPointer) {
    out.printColor(OutStream::Color::Blue, "typedef ");
    prog.getTypes().get(base).print(state);
    out << "(*" << prog.getIdents().getName(id) << ")(";
    bool first = true;
    for (TypeRef arg : args) {
      if (first)
        first = false;
      else
        out << ", ";
      prog.getTypes().get(arg).print(state);
    }
    out << ");\n";
  }

  if (getKind() == Kind::Array) {
    out.printColor(OutStream::Color::Blue, "typedef ");
    prog.getTypes().get(base).print(state);
    out << " " << prog.getIdents().getName(id);
    out << "[" << std::to_string(arraySize) << "];\n";
  }
}

bool Type::expectsVarInitializer(TypeTable &types) const {
  if (getKind() == Kind::Const)
    return true;
  if (getKind() == Kind::Array || getKind() == Kind::Volatile)
    return types.get(base).expectsVarInitializer(types);
  return false;
}

std::string Type::getDebugStr(const Program &p) const {
  OutString str;
  PrintState state(p, str);
  print(state);
  return str.getStr();
}
