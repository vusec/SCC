#include "scc/mutator-utils/GeneratorUtils.h"

Function *GeneratorUtils::addMain(Program &p) {
  TypeTable &types = p.getTypes();
  IdentTable &idents = p.getIdents();
  std::vector<Variable> mainArgs;

  const TypeRef char_p = types.getOrCreateDerived(idents, Type::Kind::Pointer,
                                                  p.getBuiltin().default_char);
  const TypeRef char_pp =
      types.getOrCreateDerived(idents, Type::Kind::Pointer, char_p);

  mainArgs.push_back(
      Variable(p.getBuiltin().signed_int, idents.createID("argc", true)));
  mainArgs.push_back(Variable(char_pp, idents.createID("argv", true)));

  auto main = std::make_unique<Function>(
      p.getBuiltin().signed_int, idents.createID("main", true), mainArgs);
  main->setBody(Statement::CompoundStmt({}));
  return &p.add(std::move(main));
}
