#include "scc/program/GlobalVar.h"
#include "scc/program/Program.h"

void GlobalVar::printForwardDecl(PrintState &state) const {}

void GlobalVar::print(PrintState &state) const {
  if (is_static)
    state.getOut().printColor(OutStream::Color::BoldBlue, "static ");

  state.getProgram().getTypes().get(type).print(state);
  state.getOut() << " " << getName(state.getProgram());

  if (initializer.getKind() != Statement::Kind::Empty) {
    state.getOut() << " = ";
    initializer.print(state);
  }

  state.getOut() << ";\n";
}

SourceDependencies GlobalVar::getDependencies(const Program &p) const {
  SourceDependencies result;
  result.add(SourceDependency(type));
  return result;
}
