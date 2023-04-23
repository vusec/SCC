#ifndef BUILTINTYPES_H
#define BUILTINTYPES_H

#include "scc/program/Builtin.h"
#include "scc/program/Type.h"

/// Utility class that creates all the builtin/primitive types in a program.
class BuiltinTypes {

  TypeRef const_void;

public:
  BuiltinTypes(Program &p);

  // List of builtin types.

  /// void
  TypeRef void_type;
  /// void *
  TypeRef void_ptr;

  /// const void*
  TypeRef const_void_ptr;
  /// volatile void *
  TypeRef volatile_void_ptr;
  /// const volatile void *
  TypeRef const_volatile_void_ptr;

  /// const char *
  TypeRef const_char_ptr;
  /// char *
  TypeRef char_ptr;

  /// char
  TypeRef default_char;
  /// unsigned char
  TypeRef unsigned_char;
  /// unsigned short
  TypeRef unsigned_short;
  /// unsigned int
  TypeRef unsigned_int;
  /// unsigned long
  TypeRef unsigned_long;
  /// unsigned long long
  TypeRef unsigned_long_long;
  /// unsigned int128_t
  TypeRef unsigned_int128;
  /// signed char
  TypeRef signed_char;
  /// signed short
  TypeRef signed_short;
  /// signed int
  TypeRef signed_int;
  /// signed long
  TypeRef signed_long;
  /// signed long long
  TypeRef signed_long_long;
  /// signed int128_t
  TypeRef signed_int128;
  /// float
  TypeRef float_type;
  /// double
  TypeRef double_type;
  /// long double
  TypeRef long_double_type;

  /// Returns true if this is the special void type.
  bool isVoid(TypeRef t) const { return t == void_type || t == const_void; }

  /// Returns true if this is any form of (unqualified) void pointer.
  bool isVoidPtr(TypeRef t) const {
    return t == void_ptr || t == volatile_void_ptr || t == const_void_ptr ||
           t == const_volatile_void_ptr;
  }

  /// Returns true if this is a builtin type defined by this class.
  bool isBuiltin(TypeRef t) const {
    return isIntType(t) || isFloatType(t) || t == void_type || t == char_ptr ||
           t == const_char_ptr || t == const_void || isVoidPtr(t);
  }

  /// Return the type that is equivalent to size_t.
  TypeRef getSizeT() const { return unsigned_long_long; }

  /// Returns a list of integer types.
  std::vector<TypeRef> getIntTypes() const {
    std::vector<TypeRef> result = {
        default_char,  unsigned_char,      unsigned_short,   unsigned_int,
        unsigned_long, unsigned_long_long, signed_char,      signed_short,
        signed_int,    signed_long,        signed_long_long, signed_int128,
    };
    if (unsigned_int128 != Void())
      result.push_back(unsigned_int128);
    return result;
  }

  /// Returns a list of signed integer types.
  std::vector<TypeRef> getSignedIntTypes() const {
    return {signed_char, signed_short,     signed_int,
            signed_long, signed_long_long, signed_int128};
  }

  /// Returns a list of unsigned integer types.
  std::vector<TypeRef> getUnsignedIntTypes() const {
    std::vector<TypeRef> result = {
        unsigned_char, unsigned_short,     unsigned_int,
        unsigned_long, unsigned_long_long,
    };
    if (unsigned_int128 != Void())
      result.push_back(unsigned_int128);
    return result;
  }

  /// Returns the floating point precision types.
  std::vector<TypeRef> getFloatTypes() const {
    return {float_type, double_type, long_double_type};
  }

  /// Returns true if this is an integer type.
  bool isIntType(TypeRef t) const {
    const auto i = getIntTypes();
    return std::find(i.begin(), i.end(), t) != i.end();
  }

  /// Returns true if this is an unsigned integer type.
  bool isUnsignedIntType(TypeRef t) const {
    const auto i = getUnsignedIntTypes();
    return std::find(i.begin(), i.end(), t) != i.end();
  }

  /// Returns true if this is a signed integer type.
  bool isSignedIntType(TypeRef t) const {
    const auto i = getSignedIntTypes();
    return std::find(i.begin(), i.end(), t) != i.end();
  }

  /// Returns true if this is a floating point type.
  bool isFloatType(TypeRef t) const {
    const auto i = getFloatTypes();
    return std::find(i.begin(), i.end(), t) != i.end();
  }

  /// Returns true if a literal of this type always needs a cast.
  ///
  /// E.g., there is no 'unsigned short' literal in C/C++.
  bool literalNeedsCast(TypeRef t) const {
    return t == default_char || t == unsigned_char || t == signed_char ||
           t == unsigned_short || t == signed_short;
  }

  std::string getIntLiteralSuffix(const Program &p, TypeRef t) const;
};

#endif // BUILTINTYPES_H
