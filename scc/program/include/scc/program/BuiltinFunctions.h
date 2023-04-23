#ifndef BUILTINFUNCTIONS_H
#define BUILTINFUNCTIONS_H

class Program;

#include "scc/program/Function.h"

/// Represents known builtin functions of a language.
///
/// Note that 'builtin' means 'provided by the system' or 'not defined by the
/// user'. E.g., `printf` is considered builtin because its provided by the
/// standard library.
struct BuiltinFunctions {
  /// Enum for all known builtin functions.
  enum class Kind {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE) ID,
#include "BuiltinFunctions.def"
  };

  /// Returns a list of all known builtin functions.
  static std::vector<Kind> getAllKinds() {
    return {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE) Kind::ID,
#include "BuiltinFunctions.def"
    };
  }

  /// Returns true if the program somehow uses the given builtin function.
  bool usesBuiltin(const Program &p, Kind k) const;

  /// Returns the return type of the given function.
  TypeRef getReturnType(Program &p, Kind k);

  /// Returns the function decl for the given builtin.
  ///
  /// This might create a declaration.
  Function *get(Program &p, Kind kind);

  /// Returns the builtin function kind for a given function name.
  ///
  /// NOTE: Only use this for testing.
  static std::optional<Kind> getKindForID(std::string name);

private:
  /// Returns the existing function decl for the given function.
  Function *getExisting(Program &p, Kind kind);

  std::unique_ptr<Function> create(Program &p, Kind kind);
  NameID getName(IdentTable &i, Kind k);
};

#endif // BUILTINFUNCTIONS_H
