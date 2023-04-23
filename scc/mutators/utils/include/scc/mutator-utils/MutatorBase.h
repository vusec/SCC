#ifndef MUTATORBASE_H
#define MUTATORBASE_H

#include "Rng.h"
#include "StrategyInstance.h"
#include "scc/program/BuiltinFunctions.h"
#include "scc/program/BuiltinTypes.h"
#include "scc/program/Program.h"
#include "scc/program/Statement.h"
#include "scc/utils/Maybe.h"
#include <iostream>

/// Optional statement.
typedef std::optional<Statement> OptStmt;
typedef Maybe<Statement> StmtOrErr;

/// Base class for mutators.
///
/// Mostly provides commonly shared data and functions.
template <class Strategy> struct MutatorBase {

  typedef StrategyInstance<Strategy> StratInst;

  /// Shared data between different mutator parts.
  struct MutatorData {
    /// The program being mutated.
    Program &p;
    /// The strategy being used to mutate the program.
    StratInst &s;
    /// The Rng seed that is being used.
    size_t seed;
    MutatorData(Program &p, StratInst &s, size_t seed)
        : p(p), s(s), seed(seed) {}
  };

  MutatorBase(MutatorData &i)
      : p(i.p), idents(i.p.getIdents()), types(i.p.getTypes()),
        builtin(i.p.getBuiltin()), builtinFuncs(i.p.getBuiltinFuncs()),
        currentRng(/*this is a dummy value updated below*/ 0), strategy(i.s),
        mutatorData(i.p, i.s, i.seed) {
    // Pick a different seed. This way multiple sub-generators within the same
    // generator run don't produce the same pattern. E.g., multiple statement
    // generators might produce the same pattern even though they were asked
    // to fill out different function bodies.
    ++i.seed;
    currentRng = Rng(i.seed);
  }

  auto getTakenDecisions() const { return strategy.getTakenDecisions(); }

protected:
  Rng currentRng;
  /// The program that is being mutated.
  Program &p;
  /// The identifier table of the program being mutated.
  IdentTable &idents;
  /// The type table of the program being mutated.
  TypeTable &types;
  /// The builtin types of the program being mutated.
  const BuiltinTypes &builtin;
  /// The builtin functions of the program being mutated.
  BuiltinFunctions &builtinFuncs;

  /// The rng seed.
  size_t seed = 0;
  typedef typename Strategy::Frag Frag;

  /// The strategy that should be used to mutate the program.
  StratInst &strategy;

  using StmtKind = Statement::Kind;
  typedef Statement Stmt;

  /// Shared data between different parts of the mutator.
  MutatorData mutatorData;

  /// Explicit return value for functions to communicate if they changed
  /// the program or not.
  enum class Modified { Yes, No };

  /// Returns the current random number generator.
  Rng &getRng() { return currentRng; }

  /// Let the strategy make a decision on the given action.
  bool decision(Frag f) { return strategy.decision(f); }

  /// Let the strategy pick a random action based on the provided weights.
  Frag pick(std::vector<Frag> l) { return strategy.pick(l); }

  /// Returns a new identifier.
  IdentTable::NameID newID(std::string prefix = "i") {
    return idents.makeNewID(prefix);
  }

  /// Returns the type references by the given TypeRef.
  const Type &getType(TypeRef t) { return types.get(t); }

  /// Strips CV-qualifiers from the given type.
  TypeRef stripCV(TypeRef t) {
    const Type &resolved = getType(t);
    if (resolved.isCVQualified())
      return stripCV(resolved.getBase());
    return t;
  }

  /// Returns true if the given type is a valid return type.
  bool isValidReturnType(TypeRef t) {
    // C was designed in hell so arrays are not valid.
    return getType(t).getKind() != Type::Kind::Array;
  }

  /// Takes the given expression and wraps it into a StmtExpr.
  Statement wrapExprInStmt(Statement c) {
    assert(c.isExpr());
    return Statement::StmtExpr(c);
  }

  /// If the given statement is actually an expression, wrap it in a StmtExpr.
  Statement ensureStmt(const Statement &s) {
    if (s.isExpr()) {
      return wrapExprInStmt(s);
    }
    return s;
  }

  [[noreturn]] void fatalError(std::string msg) {
    std::cerr << "Encountered fatal error: " << msg << "\n";
    OutString out(/*fancy=*/true);
    p.print(out).assumeSuccess("Failed to print program");
    std::cerr << "Program:\n" << out.getStr();
    abort();
  }
};

#endif // MUTATORBASE_H
