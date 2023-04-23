#include "scc/program/BuiltinTypes.h"
#include "scc/program/Program.h"

static TypeRef createIntType(Program &p, std::string name, ByteSize size,
                             bool isSigned) {
  Type t(p.getIdents().createID(name, /*fixed=*/true), size);
  t.setIsSigned(isSigned);
  return p.getTypes().addType(t);
}

static TypeRef createFloatType(Program &p, std::string name, ByteSize size) {
  return p.getTypes().addType(
      Type(p.getIdents().createID(name, /*fixed=*/true), size));
}

BuiltinTypes::BuiltinTypes(Program &p) {
  void_type = p.getTypes().addType(
      Type(p.getIdents().createID("void", /*fixed=*/true), 0));
  assert(void_type == Void() && "void not first type?");
  void_ptr = p.getTypes().getOrCreateDerived(p.getIdents(), Type::Kind::Pointer,
                                             void_type);
  const_void = p.getTypes().getOrCreateDerived(p.getIdents(), Type::Kind::Const,
                                               void_type);
  const_void_ptr = p.getTypes().getOrCreateDerived(
      p.getIdents(), Type::Kind::Pointer, const_void);

  default_char = createIntType(p, "char", 1, /*isSigned=*/true);

  char_ptr = p.getTypes().getOrCreateDerived(p.getIdents(), Type::Kind::Pointer,
                                             default_char);

  TypeRef const_char = p.getTypes().getOrCreateDerived(
      p.getIdents(), Type::Kind::Const, default_char);
  const_char_ptr = p.getTypes().getOrCreateDerived(
      p.getIdents(), Type::Kind::Pointer, const_char);

  unsigned_char = createIntType(p, "unsigned char", 1, /*isSigned=*/false);
  unsigned_short = createIntType(p, "unsigned short", 2, /*isSigned=*/false);
  unsigned_int = createIntType(p, "unsigned int", 4, /*isSigned=*/false);
  unsigned_long = createIntType(p, "unsigned long", 8, /*isSigned=*/false);
  unsigned_long_long =
      createIntType(p, "unsigned long long", 8, /*isSigned=*/false);
  if (p.getLangOpts().hasUInt128())
    unsigned_int128 = createIntType(p, "__uint128_t", 16, /*isSigned=*/false);
  signed_char = createIntType(p, "signed char", 1, /*isSigned=*/true);
  signed_short = createIntType(p, "short", 2, /*isSigned=*/true);
  signed_int = createIntType(p, "int", 4, /*isSigned=*/true);
  signed_long = createIntType(p, "long", 8, /*isSigned=*/true);
  signed_long_long = createIntType(p, "long long", 8, /*isSigned=*/true);
  signed_int128 = createIntType(p, "__int128_t", 16, /*isSigned=*/true);

  float_type = createFloatType(p, "float", 4);
  double_type = createFloatType(p, "double", 8);
  long_double_type = createFloatType(p, "long double", 8);
}

std::string BuiltinTypes::getIntLiteralSuffix(const Program &p,
                                              TypeRef t) const {
  t = p.getTypes().stripCV(t);
  if (t == unsigned_int)
    return "U";
  if (t == unsigned_long)
    return "UL";
  if (t == unsigned_long_long)
    return "ULL";
  if (t == signed_long)
    return "L";
  if (t == signed_long_long)
    return "LL";
  return "";
}
