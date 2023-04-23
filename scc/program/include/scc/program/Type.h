#pragma once

#include <cassert>
#include <limits>

#include "scc/program/Builtin.h"
#include "scc/program/IdentTable.h"
#include "scc/program/PrintState.h"
#include "scc/utils/OutStream.h"

class TypeTable;
class Program;

/// Represents any type in a program.
class Type {
  friend class TypeTable;

public:
  /// Different ways how a type can be derived.
  enum class Kind {
    /// Invalid type.
    Invalid,
    /// Not derived, e.g. just a plain `int`.
    Basic,
    /// A pointer type, e.g. `int *` or `int **`.
    Pointer,
    /// A struct or class type.
    Record,
    /// An array type.
    Array,
    /// A function pointer type.
    FunctionPointer,
    /// A const-qualified type.
    Const,
    /// A volatile-qualified type.
    Volatile,
  };

  /// Creates an invalid placeholder type.
  Type() = default;
  Type(IdentTable::NameID id, ByteSize size)
      : id(id), size(size), kind(Kind::Basic) {}
  Type(IdentTable &idents, const TypeTable &types, Kind d, TypeRef base);

  /// Creates a record type pointing to the record with the given name.
  static Type Record(NameID recordID) {
    Type t;
    t.kind = Kind::Record;
    t.id = recordID;
    return t;
  }

  /// Creates a function pointer type with the given return/arg types and
  /// an alias that it is referred to in the rest of the program.
  ///
  /// Note: The alias is used because the type is typedef'd.
  static Type FunctionPointer(TypeRef returnType, std::vector<TypeRef> args,
                              NameID alias) {
    Type t;
    t.kind = Kind::FunctionPointer;
    t.base = returnType;
    t.id = alias;
    t.args = args;
    return t;
  }

  /// Creates a new array type with the given base type and array size.
  ///
  /// Note: The alias is used because the type is typedef'd.
  static Type Array(TypeRef base, unsigned size, NameID alias) {
    Type t;
    t.kind = Kind::Array;
    t.arraySize = size;
    t.base = base;
    t.id = alias;
    return t;
  }

  /// Creates a const-qualified version of the given type.
  static Type Const(TypeRef base) {
    Type t;
    t.kind = Kind::Const;
    t.base = base;
    return t;
  }

  /// Removes any outer types of the given kinds.
  ///
  /// This e.g. can remove the 'const' from a 'const int' (and returns 'int').
  const Type &strip(const TypeTable &t, std::vector<Kind> toStrip) const;

  /// Returns if this type itself is volatile.
  ///
  /// Note: It might be a volatile type of a const type.
  bool isConst() const { return kind == Kind::Const; }

  /// Returns if this type itself is const.
  ///
  /// Note: It might be a const type of a volatile type.
  bool isVolatile() const { return kind == Kind::Volatile; }

  /// Returns true if this type has any direct CV-qualifiers.
  bool isCVQualified() const { return isConst() || isVolatile(); }

  /// Returns true if this is a sized type.
  ///
  /// E.g., 'int' is sized, but a struct type with only a forward declaration
  /// is not sized.
  bool hasSize() const { return size != 0; }

  /// If this is a pointer type.
  bool isPointer() const { return kind == Kind::Pointer; }
  /// If this is an array type.
  bool isArray() const { return kind == Kind::Array; }
  /// If this is an array or pointer type.
  bool isPointerOrArray() const { return isPointer() || isArray(); }
  /// If this is a record type.
  bool isRecord() const { return kind == Kind::Record; }

  /// If this is a record, returns the name of its declaration.
  NameID getRecordNameID() const {
    SCCAssert(isRecord(), "Must be a record");
    return id;
  }

  /// If this is a sized type, return its byte size.
  ByteSize getByteSize() const {
    SCCAssert(hasSize(), "getByteSize called on unsized type?");
    return size;
  }
  /// If this is a sized type, returns its bit size.
  unsigned getBitSize() const { return getByteSize() * 8U; }

  /// Sets whether this is a signed true (e.g., 'signed int').
  ///
  /// Only callable on basic types.
  void setIsSigned(bool s) {
    SCCAssert(getKind() == Kind::Basic, "only basic types have signs");
    isSignedType = s;
  }

  /// If this is a signed basic type.
  ///
  /// Only callable on basic types.
  bool isSigned() const {
    SCCAssert(getKind() == Kind::Basic, "only basic types have signs");
    return isSignedType;
  }

  const std::vector<TypeRef> &getArgs() const {
    SCCAssert(getKind() == Kind::FunctionPointer, "Only valid on func ptrs");
    return args;
  }

  TypeRef getFuncReturnType() const {
    SCCAssert(getKind() == Kind::FunctionPointer, "Only valid on func ptrs");
    return base;
  }

  TypeRef getRef() const { return ref; }

  void print(PrintState &state) const;

  void printPreamble(PrintState &state) const;

  bool isDerived() const { return kind != Kind::Basic; }

  TypeRef getBase() const {
    SCCAssert(isDerived(), "getBase only works on derived types.");
    return base;
  }

  Kind getKind() const { return kind; }

  /// If this is an array type, returns the underlying array size.
  unsigned getArraySize() const {
    SCCAssert(getKind() == Kind::Array, "Expected array");
    return arraySize;
  }

  /// If this is an array type, sets the underlying array size.
  void setArraySize(unsigned n) {
    SCCAssert(getKind() == Kind::Array, "Expected array");
    arraySize = n;
  }

  /// Sets the 'base' type this is derived from.
  ///
  /// Only makes sense to call this on e.g. array types.
  void setBase(TypeRef b) {
    SCCAssert(isDerived(), "Only derived types have bases");
    base = b;
  }

  /// If this type references the given string.
  ///
  /// Note: Types don't reference themselves.
  bool usesType(TypeRef t) const {
    if (base == t)
      return true;
    for (TypeRef arg : args)
      if (arg == t)
        return true;

    return false;
  }

  bool isValid() const { return kind != Kind::Invalid; }

  /// True if C/C++ expects this type to be always initialized.
  ///
  /// E.g., it's not allowed to declare a 'const int var123;'.
  bool expectsVarInitializer(TypeTable &types) const;

  std::string getDebugStr(const Program &p) const;

private:
  /// The identifier by which this type is identified.
  /// Note: For derived types this is nothing.
  IdentTable::NameID id = InvalidName;
  /// The size of this type.
  /// FIXME: Not used at the moment.
  ByteSize size = 0;
  bool isSignedType = false;
  unsigned arraySize = 0;
  /// The TypeRef value that identifies this type.
  TypeRef ref = Void();
  /// The base type if this is a derived type.
  /// For `int *` this points to `int`.
  TypeRef base = Void();
  /// How this type was derived.
  Kind kind = Kind::Invalid;

  std::vector<TypeRef> args;
};
