#include "scc/program/Function.h"
#include "scc/program/Program.h"

void Function::printPrefix(PrintState &state, bool isDef) const {
  OutStream &out = state.getOut();
  const Program &prog = state.getProgram();

  if (!callingConv.empty())
    out << callingConv << " ";
  switch (weight) {
  case Weight::None:
    break;
  case Weight::Hot:
    out << "__attribute__((cold))"
        << " ";
    break;
  case Weight::Cold:
    out << "__attribute__((hot))"
        << " ";
    break;
  }

  if (isDef)
    for (const std::string &attr : attributes)
      out << " " << attr << " ";
  if (!attributes.empty())
    out << "\n";

  if (isStatic)
    out << "static ";
  prog.getTypes().get(returnType).print(state);
  out << " " << getName(prog) << "(";

  bool first = true;
  for (Variable v : args) {
    if (first)
      first = false;
    else
      out << ", ";

    prog.getTypes().get(v.getType()).print(state);
    out << " " << prog.getIdents().getName(v.getName());
  }
  if (variadic == Variadic::Yes) {
    out << ", ...";
  }
  out << ")";

  if (isNoExcept)
    out << " noexcept ";
}

void Function::printIncludes(PrintState &state) const {
  if (isExternal())
    state.printHeaderInclude(externalHeader);
}

void Function::printForwardDecl(PrintState &state) const {
  if (isExternal())
    return;
  printPrefix(state, false);
  state.getOut() << ";\n";
}

void Function::print(PrintState &state) const {
  if (isExternal())
    return;
  assert(this->getName(state.getProgram()) != "malloc");

  printPrefix(state, true);
  if (!body.isEmpty()) {
    if (body != Statement::Kind::Compound)
      Statement::CompoundStmt({body}).print(state);
    else
      body.print(state);
  } else
    state.getOut() << ";";
  state.getOut() << "\n";
}

bool Function::usesType(TypeRef t) const {
  if (returnType == t)
    return true;

  for (Variable v : args) {
    if (v.getType() == t)
      return true;
  }
  return body.usesType(t);
}

bool Function::referencesID(NameID id) const { return body.usesID(id); }

size_t Function::countNodes() const {
  size_t res = 1U + args.size() + attributes.size() * 5 + body.countNodes();
  return res;
}

SourceDependencies Function::getDependencies(const Program &p) const {
  SourceDependencies result;
  result.add(SourceDependency(returnType));
  for (const auto &arg : args)
    result.add(SourceDependency(arg.getType()));

  result.merge(SourceDependencies::fromStmt(p, body));

  return result;
}

bool Function::isMain(const Program &p) const {
  return getName(p.getIdents()) == "main";
}
