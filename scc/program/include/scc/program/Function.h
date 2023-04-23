#pragma once

#include <set>

#include "scc/program/NamedDecl.h"
#include "scc/program/Scope.h"
#include "scc/program/Statement.h"
#include "scc/program/Type.h"
#include "scc/program/Variable.h"
#include "scc/utils/Maybe.h"

/// Represents a function in our C program.
class Function : public NamedDecl {
public:
  std::string callingConv;
  enum class Weight { Hot, Cold, None };
  Weight weight = Weight::None;

  void printPrefix(PrintState &state, bool isDef) const;

public:
  enum class Variadic { Yes, No };
  explicit Function(TypeRef r, IdentTable::NameID name,
                    std::vector<Variable> args,
                    Variadic variadic = Variadic::No)
      : NamedDecl(Decl::Kind::Function, name), returnType(r), args(args),
        variadic(variadic) {}

  bool isStatic = false;
  bool isNoExcept = false;

  Decl *clone() const override {
    Function *f = new Function(*this);
    return f;
  }

  void setExternalHeader(std::string h) { externalHeader = h; }

  bool isExternal() const { return !externalHeader.empty(); }

  void setCallingConv(std::string c) { callingConv = c; }

  bool setWeight(Weight w) {
    bool changed = w != weight;
    weight = w;
    return changed;
  }

  void addAttr(std::string attr) { attributes.insert(attr); }

  std::vector<std::string> getAllAttrs() const {
    std::vector<std::string> res;
    for (const auto &attr : attributes)
      res.push_back(attr);
    return res;
  }

  void removeAttr(std::string attr) { attributes.erase(attr); }

  TypeRef getReturnType() const { return returnType; }

  void printIncludes(PrintState &state) const override;

  void printForwardDecl(PrintState &state) const override;

  void print(PrintState &state) const override;

  const std::vector<Variable> &getArgs() const { return args; }

  Maybe<Variable> getArg(NameID id) const {
    for (const Variable &arg : args)
      if (arg.getName() == id)
        return arg;
    return Err("Could not find variable");
  }

  void setBody(Statement body) { this->body = std::move(body); }

  Statement &getBody() { return body; }

  const Statement &getBody() const { return body; }

  /// Returns true if this function somehow references the given type.
  bool usesType(TypeRef t) const override;

  /// Returns true if this function somehow uses the given identifier.
  bool referencesID(NameID id) const override;

  size_t countNodes() const override;

  /// Returns the source dependencies to print this function.
  SourceDependencies getDependencies(const Program &p) const override;

  void verifySelf(const Program &p) const override { body.verifySelf(p); }

  bool isMain(const Program &p) const;

  Statement makeCall(std::vector<Statement> args) const {
    return Statement::Call(returnType, getNameID(), args);
  }

  bool isVariadic() const { return variadic == Variadic::Yes; }

private:
  /// The return type of this function.
  TypeRef returnType;

  /// The function body.
  Statement body;

  /// Whether this function is variadic.
  Variadic variadic = Variadic::No;

  /// The argument name/types of this function.
  std::vector<Variable> args;

  /// Name of the external header providing this function. Empty string if this
  /// is a function defined in this program.
  std::string externalHeader;

  /// List of attributes for this function.
  std::set<std::string> attributes;
};
