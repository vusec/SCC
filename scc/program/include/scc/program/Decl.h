#pragma once

#include "scc/program/Builtin.h"
#include "scc/program/Dependencies.h"
#include "scc/program/IdentTable.h"
#include "scc/program/PrintState.h"
#include "scc/utils/CopyableUniquePtr.h"

#include <memory>

class Program;

/// Extra data associated with declarations.
struct DeclExtraData {
  virtual ~DeclExtraData();
  virtual std::unique_ptr<DeclExtraData> clone() const = 0;
  virtual void dump(const Program &p) const = 0;
  virtual std::string getSummary(const Program &p) const = 0;
};

/// Represents a declaration the target program. Can be a function, global
/// or class/struct declaration.
///
/// Note that this does not necessarily map to a specific declaration
/// within a program. E.g., an external function from the C standard library
/// is a Decl, but its only represented in the program as an include
/// directive.
class Decl {
public:
  /// Ad-hoc RTTI support.
  enum class Kind {
    Function,
    GlobalVar,
    Record,
  };

  explicit Decl(Kind k) : kind(k) {}

  Kind getKind() const { return kind; }

  bool isNamed() const { return true; }

  virtual ~Decl();
  /// Returns a copy of this Decl.
  virtual Decl *clone() const = 0;

  /// Prints any includes needed for this decl.
  virtual void printIncludes(PrintState &state) const {}

  /// Prints the C forward declaration of this decl.
  virtual void printForwardDecl(PrintState &state) const = 0;

  /// Prints the C representation of this decl.
  virtual void print(PrintState &state) const = 0;

  virtual bool usesType(TypeRef t) const = 0;

  virtual bool referencesID(NameID id) const = 0;

  virtual size_t countNodes() const = 0;

  virtual SourceDependencies getDependencies(const Program &p) const = 0;

  virtual void verifySelf(const Program &p) const = 0;

  const DeclExtraData *getExtraData() const { return extraData.data.get(); }
  void setExtraData(std::unique_ptr<DeclExtraData> &&d) {
    extraData.data = std::move(d);
  }
  void removeExtraData() { extraData.data.reset(); }

  void dump(const Program &p) const;

private:
  /// \see Kind
  Kind kind;
  CopyableUniquePtr<DeclExtraData> extraData;
};
