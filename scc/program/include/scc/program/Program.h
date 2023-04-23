#pragma once

#include "scc/program/Builtin.h"
#include "scc/program/BuiltinFunctions.h"
#include "scc/program/BuiltinTypes.h"
#include "scc/program/DeclStorage.h"
#include "scc/program/Function.h"
#include "scc/program/IdentTable.h"
#include "scc/program/LangOpts.h"
#include "scc/program/TypeTable.h"
#include "scc/utils/Error.h"
#include "scc/utils/OutStream.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Record;

/// Represents a program.
///
/// Note that this is *not* representing a single translation unit.
class Program {
  /// List of known identiifers in this program.
  IdentTable idents;
  /// List of types in this program.
  TypeTable types;
  /// List of declarations in this program.
  DeclStorage global;
  /// List of builtin functions.
  BuiltinFunctions builtinFuncs;

  /// The language options of this program.
  LangOpts opts;

  // Declared last because it references LangOpts.
  /// List of builtin types.
  BuiltinTypes builtin;

  void createIntType(std::string name, ByteSize size, bool isSigned);

public:
  Program();

  /// Prints a textual representation of the program to the given stream.
  ///
  /// Returns an error if printing failed. This can happen if e.g., there
  /// are impossible types in the program. One example for such a type is
  /// `struct X { struct X x; }` which recursively depends on itself.
  [[nodiscard]] OptError print(OutStream &s) const;

  /// Returns true if the program can be printed without an error
  ///
  /// @see Program::print
  bool canPrint() const;

  /// Add the declaration to the proram and returns a reference to the stored
  /// declaration.
  template <typename T> T &add(std::unique_ptr<T> &&n) {
    return static_cast<T &>(global.store(n.release()));
  }

  std::vector<DeclStorage *> getDeclStorages() { return {&global}; }

  /// Returns the list of all stored declarations.
  auto getDeclList() const { return global.asVec(); }

  /// Removes a declaration from the program.
  void removeDecl(Decl *d) { global.remove(d); }

  const BuiltinTypes &getBuiltin() const { return builtin; }

  BuiltinFunctions &getBuiltinFuncs() { return builtinFuncs; }
  const BuiltinFunctions &getBuiltinFuncs() const { return builtinFuncs; }

  IdentTable &getIdents() { return idents; }
  TypeTable &getTypes() { return types; }
  const IdentTable &getIdents() const { return idents; }
  const TypeTable &getTypes() const { return types; }

  bool isTypeUsed(TypeRef t) const;
  bool isIDUsed(NameID id) const;

  void dump(bool fancy = true) const;
  std::string toDebugStr() const;

  size_t countNodes() const;

  void setLangOpts(LangOpts o) { opts = o; }

  const LangOpts &getLangOpts() const { return opts; }

  /// Returns the function with the given name.
  Function *getFunctionWithID(NameID id);

  /// Returns the Record for a given type.
  const Record &getRecord(TypeRef t) const;

  /// Finds a Decl by name.
  ///
  /// Aborts if the Decl can't be found.
  const Decl &lookup(NameID id) const;

  /// Performs an integrity check on the program and aborts the program if
  /// this fails.
  ///
  /// Possible ways this check can fail if e.g., there is a syntax error in
  /// the code or an undefined type is used.
  ///
  /// This should be used between mutations to help with debugging.
  void verifySelf() const {
#ifndef NDEBUG
    for (const Decl *d : getDeclList())
      d->verifySelf(*this);
#endif
  }

  struct VerifyOnExit {
    const Program *p;
    explicit VerifyOnExit(const Program &p) : p(&p) { p.verifySelf(); }
    ~VerifyOnExit() { p->verifySelf(); }
  };
  /// Returns an object that on destruction/scope-exit validates the current
  /// program.
  VerifyOnExit queueVerify() const { return VerifyOnExit(*this); }
};
