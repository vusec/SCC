#include "scc/program/DeclStorage.h"
#include "scc/program/Dependencies.h"
#include "scc/program/Program.h"

#include <array>

DeclStorage &DeclStorage::operator=(const DeclStorage &o) {
  namedDecls.clear();
  decls.clear();
  for (const auto &d : o.decls)
    store(static_cast<NamedDecl *>(d->clone()));
  return *this;
}

OptError DeclStorage::print(PrintState &state) const {
  // Print all the includes. These are already filtered by already printed
  // includes from the printer.
  if (!state.getOut().isInformalOutput())
    for (const auto &d : decls)
      d->printIncludes(state);

  SourceDependencies deps;
  for (const auto &d : decls)
    deps.add(SourceDependency(*d));

  const Program &p = state.getProgram();
  Maybe<std::vector<SourceDependency>> list = deps.getOrdered(p);
  if (list.isErr())
    return list.takeError();

  for (const SourceDependency &dep : *list) {
    switch (dep.getKind()) {
    case SourceDependency::Kind::Decl:
      dep.getDecl().print(state);
      break;
    case SourceDependency::Kind::Type:
      state.getProgram().getTypes().get(dep.getType()).printPreamble(state);
      break;
    }
  }

  return {};
}

std::vector<const Decl *> DeclStorage::getFilteredDecls(Decl::Kind kind) const {
  std::vector<const Decl *> result;

  for (auto i = decls.rbegin(); i != decls.rend(); ++i)
    if ((*i)->getKind() == kind)
      result.push_back(i->get());

  return result;
}
