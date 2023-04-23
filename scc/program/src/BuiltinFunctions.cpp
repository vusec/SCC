#include "scc/program/BuiltinFunctions.h"
#include "scc/program/Program.h"

bool BuiltinFunctions::usesBuiltin(const Program &p, Kind k) const {
  const IdentTable &i = p.getIdents();
  switch (k) {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE)                                  \
  case Kind::ID:                                                               \
    return i.hasID(#NAME);
#include "scc/program/BuiltinFunctions.def"
  }
  SCCError("Missing switch?");
}

TypeRef BuiltinFunctions::getReturnType(Program &p, Kind k) {
  const BuiltinTypes &b = p.getBuiltin();
  auto void_ptr = p.getBuiltin().void_ptr;
  switch (k) {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE)                                  \
  case Kind::ID:                                                               \
    return RET_TYPE;
#include "scc/program/BuiltinFunctions.def"
  }
  SCCError("Missing switch?");
}

Function *BuiltinFunctions::getExisting(Program &p, Kind kind) {
  NameID id = getName(p.getIdents(), kind);
  for (Decl *d : p.getDeclList()) {
    if (d->getKind() == Decl::Kind::Function) {
      Function *f = static_cast<Function *>(d);
      if (f->getNameID() == id)
        return f;
    }
  }
  return nullptr;
}

using Kind = BuiltinFunctions::Kind;

static void addExternalHeaderRefs(Function &f, Kind k) {
  switch (k) {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE)                                  \
  case Kind::ID:                                                               \
    f.setExternalHeader(HEADER);                                               \
    break;
#include "scc/program/BuiltinFunctions.def"
  }
}

Function *BuiltinFunctions::get(Program &p, Kind kind) {
  Function *lookup = getExisting(p, kind);
  if (lookup)
    return lookup;

  std::unique_ptr<Function> res = create(p, kind);
  addExternalHeaderRefs(*res, kind);
  assert(res->isExternal());
  return &p.add(std::move(res));
}

std::optional<BuiltinFunctions::Kind>
BuiltinFunctions::getKindForID(std::string name) {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE)                                  \
  if (#NAME == name)                                                           \
    return Kind::ID;
#include "scc/program/BuiltinFunctions.def"
  return {};
}

static std::unique_ptr<Function>
makeBuiltin(NameID id, Program &p, TypeRef ret, std::vector<TypeRef> argTypes,
            Function::Variadic va = Function::Variadic::No) {
  std::vector<Variable> args;
  for (TypeRef t : argTypes)
    args.push_back(Variable(t, p.getIdents().makeNewID("a")));
  return std::make_unique<Function>(ret, id, args);
}

static std::unique_ptr<Function>
makeVABuiltin(NameID id, Program &p, TypeRef ret, std::vector<TypeRef> argTypes,
              Function::Variadic va = Function::Variadic::No) {
  return makeBuiltin(id, p, ret, argTypes, Function::Variadic::Yes);
}

std::unique_ptr<Function> BuiltinFunctions::create(Program &p, Kind k) {
  const BuiltinTypes &b = p.getBuiltin();
  NameID id = getName(p.getIdents(), k);
  TypeRef retT = getReturnType(p, k);
  switch (k) {
  case Kind::Alloca:
  case Kind::Malloc:
    return makeBuiltin(id, p, retT, {b.getSizeT()});
  case Kind::Calloc:
    return makeBuiltin(id, p, retT, {b.getSizeT(), b.getSizeT()});
  case Kind::Free:
    return makeBuiltin(id, p, retT, {b.void_ptr});
  case Kind::MemSet:
    return makeBuiltin(id, p, retT, {b.void_ptr, b.signed_int, b.getSizeT()});
  case Kind::MemChr:
    return makeBuiltin(id, p, retT,
                       {b.const_void_ptr, b.signed_int, b.getSizeT()});
  case Kind::MemCpy:
    return makeBuiltin(id, p, retT,
                       {b.void_ptr, b.const_void_ptr, b.getSizeT()});
  case Kind::StrStr:
    return makeBuiltin(id, p, retT, {b.const_char_ptr, b.const_char_ptr});
  case Kind::StrCaseStr:
    return makeBuiltin(id, p, retT, {b.const_char_ptr, b.const_char_ptr});
  case Kind::StrCpy:
    return makeBuiltin(id, p, retT, {b.char_ptr, b.const_char_ptr});
  case Kind::StrCmp:
    return makeBuiltin(id, p, retT, {b.const_char_ptr, b.const_char_ptr});
  case Kind::StrNCmp:
    return makeBuiltin(id, p, retT,
                       {b.const_char_ptr, b.const_char_ptr, b.getSizeT()});
  case Kind::StrNCpy:
    return makeBuiltin(id, p, retT,
                       {b.char_ptr, b.const_char_ptr, b.getSizeT()});
  case Kind::MemCmp:
    return makeBuiltin(id, p, retT,
                       {b.const_void_ptr, b.const_void_ptr, b.getSizeT()});
  case Kind::MemMove:
    return makeBuiltin(id, p, retT,
                       {b.void_ptr, b.const_void_ptr, b.getSizeT()});
  case Kind::Realloc:
    return makeBuiltin(id, p, retT, {b.void_ptr, b.getSizeT()});
  case Kind::Strlen:
    return makeBuiltin(id, p, retT, {b.const_char_ptr});
  case Kind::StrNlen:
    return makeBuiltin(id, p, retT, {b.const_char_ptr, b.getSizeT()});
  case Kind::Exit:
    return makeBuiltin(id, p, retT, {b.signed_int});
  case Kind::Abort:
    return makeBuiltin(id, p, retT, {});
  case Kind::Printf:
    return makeVABuiltin(id, p, retT, {b.const_char_ptr});
  case Kind::GetChar:
    return makeBuiltin(id, p, retT, {});
  }
  SCCError("Missing switch?");
}

NameID BuiltinFunctions::getName(IdentTable &i, Kind k) {
  switch (k) {
#define BUILTIN_F(ID, NAME, HEADER, RET_TYPE)                                  \
  case Kind::ID:                                                               \
    return i.getOrCreateID(#NAME, /*fixed=*/true);
#include "scc/program/BuiltinFunctions.def"
  }
  SCCError("Missing switch?");
}
