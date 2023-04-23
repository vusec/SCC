#include "scc/program/Dependencies.h"
#include "scc/program/Program.h"
#include "scc/program/RecordDecl.h"

#include <list>

namespace {

typedef std::vector<SourceDependency> DepVec;
typedef std::unordered_set<SourceDependency> DepSet;
typedef std::list<SourceDependency> DepList;

DepVec getDirectDependencies(SourceDependency dependency, const Program &p) {
  switch (dependency.getKind()) {
  case SourceDependency::Kind::Decl: {
    // For a declaration we can directly ask the Decl itself.
    DepVec result;
    const Decl &decl = dependency.getDecl();
    for (const SourceDependency &dep : decl.getDependencies(p))
      result.push_back(dep);
    return result;
  }
  case SourceDependency::Kind::Type: {
    const TypeRef type = dependency.getType();
    const Type &resolved = p.getTypes().get(type);

    DepVec result;
    switch (resolved.getKind()) {
    // Invalid/basic types like 'int' don't have any dependencies.
    case Type::Kind::Invalid:
    case Type::Kind::Basic:
      break;
    // Pointers/arrays/const depend on their base type.
    case Type::Kind::Pointer:
    case Type::Kind::Array:
    case Type::Kind::Const:
    case Type::Kind::Volatile:
      result.push_back(SourceDependency(resolved.getBase()));
      break;
    case Type::Kind::FunctionPointer:
      result.push_back(SourceDependency(resolved.getFuncReturnType()));
      for (TypeRef arg : resolved.getArgs())
        result.push_back(SourceDependency(arg));
      break;
    case Type::Kind::Record:
      result.push_back(SourceDependency(p.getRecord(type)));
      break;
    }
    return result;
  }
  }
  SCCError("Unimplemented switch?");
}

struct SourceOrder {
  struct WaitingDep {
    SourceDependency dep;
    DepVec directDeps;
  };

  Maybe<DepVec> getOrdered(const DepVec &deps) {
    for (const SourceDependency &dep : deps)
      addToWaitingList(dep);

    for (auto iter = waitingList.begin(); iter != waitingList.end(); ++iter) {
      DepVec directDeps = getDirectDependencies(iter->dep, prog);
      for (const SourceDependency &dep : directDeps)
        addToWaitingList(dep);
    }
    waitingListSet.clear();

    std::size_t iterationsWithoutProgress = 0;
    const std::size_t grace = 10;

    while (!waitingList.empty()) {
      if (iterationsWithoutProgress > waitingList.size() + grace)
        return Err("Cyclic dependency in program.");

      WaitingDep current = waitingList.front();
      waitingList.pop_front();

      if (tryEmit(current)) {
        iterationsWithoutProgress = 0;
        continue;
      }

      iterationsWithoutProgress++;
      waitingList.push_back(current);
    }

    return emitted;
  }

  explicit SourceOrder(const Program &p) : prog(p) {}

private:
  bool addToWaitingList(const SourceDependency &dep) {
    if (waitingListSet.count(dep) != 0)
      return false;
    waitingListSet.insert(dep);
    waitingList.push_back({dep, getDirectDependencies(dep, prog)});
    return true;
  }

  bool tryEmit(const WaitingDep &waiting) {
    for (const SourceDependency &dep : waiting.directDeps)
      if (alreadyEmitted.count(dep) == 0)
        return false;
    alreadyEmitted.insert(waiting.dep);
    emitted.push_back(waiting.dep);
    return true;
  }

  DepSet waitingListSet;
  std::list<WaitingDep> waitingList;
  DepSet alreadyEmitted;
  DepVec emitted;

  const Program &prog;
};
} // namespace

Maybe<std::vector<SourceDependency>>
SourceDependencies::getOrdered(const Program &p) {
  return SourceOrder(p).getOrdered(values);
}

SourceDependencies SourceDependencies::fromStmt(const Program &p,
                                                const Statement &s) {
  SourceDependencies result;
  s.foreachChild([&p, &result](const Statement &child) {
    for (TypeRef t : child.getReferencedTypes())
      result.add(SourceDependency(t));
    if (child == Statement::Kind::GlobalVarRef) {
      const Decl &decl = p.lookup(child.getReferencedVarID());
      result.add(SourceDependency(decl));
    }
    if (child == Statement::Kind::Call) {
      const Decl &decl = p.lookup(child.getCalledFuncID());
      result.add(SourceDependency(decl));
    }
    return LoopCtrl::Continue;
  });
  return result;
}
