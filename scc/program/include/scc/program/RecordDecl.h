#ifndef RECORDDECL_H
#define RECORDDECL_H

#include "scc/program/IdentTable.h"
#include "scc/program/NamedDecl.h"
#include "scc/program/TypeTable.h"
#include "scc/program/Variable.h"

/// Represents a struct/class/union declaration in our program.
class Record : public NamedDecl {
public:
  Record(Program &p, IdentTable::NameID name, bool packed = false);

  /// Represents a field in our record.
  struct Field {
    Field() = default;
    Field(NameID n, TypeRef t, std::optional<unsigned> bitfieldSize = {}) {
      name = n;
      type = t;
      bitSize = bitfieldSize;
    }
    NameID getName() const { return name; }

    bool hasName() const { return true; }

    TypeRef getType() const { return type; }

    std::optional<unsigned> getBitfieldSize() const { return bitSize; }

    bool isBitfield() const { return bitSize ? true : false; }

    Variable asVar() const { return Variable(type, name); }

    void setAlignment(unsigned align) { alignment = align; }
    unsigned getMinAlignment() const { return alignment; }

  private:
    TypeRef type = Void();
    NameID name = InvalidName;
    std::optional<unsigned> bitSize = 0;
    unsigned alignment = 1;
  };

  static std::unique_ptr<Record> Struct(Program &p, NameID name,
                                        std::vector<Field> fields) {
    auto result = std::make_unique<Record>(p, name);
    result->fields = fields;
    return result;
  }

  static std::unique_ptr<Record> Union(Program &p, NameID name) {
    auto result = std::make_unique<Record>(p, name);
    result->isAUnion = true;
    return result;
  }

  void addField(const Field &f) { fields.push_back(f); }

  void printForwardDecl(PrintState &state) const override;

  void print(PrintState &state) const override;

  bool usesType(TypeRef t) const override;

  bool referencesID(NameID id) const override;

  Decl *clone() const override { return new Record(*this); }

  SourceDependencies getDependencies(const Program &p) const override;

  void verifySelf(const Program &p) const override {}

  bool isUnion() const { return isAUnion; }
  bool isPacked() const { return packed; }
  ByteSize getMinAlignment() const { return alignment; }
  const std::vector<Field> &getFields() const { return fields; }
  TypeRef getType() const { return type; }

private:
  ByteSize alignment = 1;
  bool isAUnion = false;
  bool packed = false;
  TypeRef type;

  std::vector<Field> fields;

  void printCommonPrefix(PrintState &state) const;

  size_t countNodes() const override { return fields.size(); }
};

#endif // RECORDDECL_H
