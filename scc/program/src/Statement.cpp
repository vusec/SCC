#include "scc/program/Statement.h"
#include "scc/program/Program.h"
#include "scc/utils/OnScopeExit.h"
#include <iostream>

void Statement::verify() const {
  if (isExpr()) {
    for (const Statement &s : children) {
      assert(s.isExpr());
    }
  }
}

Statement Statement::BinaryOp(Program &p, Kind op, Statement lhs,
                              Statement rhs) {
  expectExpr(lhs);
  expectExpr(rhs);
  Statement res(op, {lhs, rhs});
  if (Statement::getComparisonOps().count(op))
    res.type = p.getBuiltin().signed_int;
  else
    res.type = lhs.type;
  return res;
}

Statement Statement::SizeOf(Program &p, TypeRef type) {
  Statement result;
  result.kind = Statement::Kind::Sizeof;
  result.otherType = type;
  result.type = p.getBuiltin().unsigned_long_long;
  return result;
}

Statement Statement::GlobalVarRef(Variable var) {
  Statement res(Kind::GlobalVarRef);
  res.id = var.getName();
  res.type = var.getType();
  return res;
}

namespace {
struct MaybeWrapInCurly {
  OutStream &out;
  bool wrap = false;
  MaybeWrapInCurly(OutStream &o, const Statement &s) : out(o) {
    wrap = s.getKind() == Statement::Kind::Group;
    if (wrap)
      out << " {";
  }
  ~MaybeWrapInCurly() {
    if (wrap) {
      out.printIndent();
      out << "}";
    }
  }
};
} // namespace

void Statement::print(PrintState &state) const {
  OutStream &out = state.getOut();
  const Program &prog = state.getProgram();

  // Print a custom prefix.
  if (const ExtraData *data = getExtraData())
    data->printPrefix(*this, state);
  // Print a custom suffix.
  OnScopeExit suffixOnExit([this, &state]() {
    if (const ExtraData *data = getExtraData())
      data->printSuffix(*this, state);
  });

  switch (kind) {
  case Kind::Compound: {
    out.printIndent();
    out << "{\n";
    {
      printSummary(state);
      IndentScope scope(out);
      printChildren(state);
    }
    out.printIndent();
    out << "}\n";
    break;
  }
  case Kind::Group:
    if (!constantValue.empty()) {
      out.printIndent();
      out.printColor(OutStream::Color::Blue,
                     "/* BEGIN: " + constantValue + "*/\n");
    }
    printChildren(state);
    printSummaryLine(state);
    if (!constantValue.empty()) {
      out.printIndent();
      out.printColor(OutStream::Color::Blue,
                     "/* END: " + constantValue + "*/\n");
    }
    break;
  case Kind::Not:
    out << "(!(";
    printChildren(state);
    out << "))";
    break;
  case Kind::Sizeof:
    out << " sizeof(";
    prog.getTypes().get(otherType).print(state);
    out << ")";
    break;
  case Kind::Cast:
    out << "((";
    prog.getTypes().get(type).print(state);
    out << ")(";
    printChildren(state);
    out << "))";
    break;
  case Kind::AttrStmt:
    out.printIndent();
    out << constantValue << " ";
    printChildren(state);
    break;
  case Kind::New: {
    out.printColor(OutStream::Color::Magenta, " new ");
    TypeRef base = prog.getTypes().get(type).getBase();
    prog.getTypes().get(base).print(state);
    out << "(";
    bool first = true;
    for (const Statement &u : children) {
      if (first)
        first = false;
      else
        out << ",";
      u.print(state);
    }
    out << ")";
    break;
  }
  case Kind::Delete:
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "delete ");
    out << " (";
    printChildren(state);
    out << ");\n";
    break;
  case Kind::Throw:
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "throw ");
    printChildren(state);
    out << ";\n";
    break;
  case Kind::Try: {
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "try");
    out << " {\n ";
    {
      IndentScope scope(out);
      children.front().print(state);
    }
    out.printIndent();
    out << "}\n";
    bool first = true;
    for (auto &c : children) {
      if (first) {
        first = false;
        continue;
      }
      c.print(state);
    }
    break;
  }
  case Kind::Catch: {
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "catch");
    out << " (";
    if (type != Void()) {
      prog.getTypes().get(type).print(state);
      out << " ";
      out << prog.getIdents().getName(id);
    } else {
      out << "...";
    }
    out << ") {\n";
    {
      IndentScope scope(out);
      printChildren(state);
    }
    out.printIndent();
    out << "}\n";
    break;
  }
  case Kind::GotoLabel:
    out.printIndent();
    out << "/*label*/";
    out << prog.getIdents().getName(id);
    out << ":;\n";
    break;
  case Kind::Goto:
    out.printIndent();
    out << "goto ";
    out << prog.getIdents().getName(id);
    out << ";\n";
    break;
  case Kind::AddrOf:
    out << "(&";
    printChildren(state);
    out << ")";
    break;
  case Kind::AddrOfFunc:
    out << "(&";
    out << prog.getIdents().getName(id);
    out << ")";
    break;
  case Kind::Deref:
    out << "(*";
    printChildren(state);
    out << ")";
    break;
  case Kind::Arrow:
    out << "((";
    children.front().print(state);
    out << ")->";
    out << prog.getIdents().getName(id);
    out << ")";
    break;
  case Kind::Dot:
    out << "((";
    children.front().print(state);
    out << ").";
    out << prog.getIdents().getName(id);
    out << ")";
    break;
  case Kind::Subscript:
    out << "((";
    children.at(0).print(state);
    out << ")[";
    children.at(1).print(state);
    out << "])";
    break;
  case Kind::Empty:
    out.printIndent();
    out << constantValue << ";\n";
    break;
  case Kind::GlobalVarRef:
    out << prog.getIdents().getName(id);
    break;
  case Kind::LocalVarRef:
    out << prog.getIdents().getName(id);
    break;
  case Kind::VarDecl:
    out.printIndent();
    prog.getTypes().get(type).print(state);
    out << " ";
    out << prog.getIdents().getName(id);
    out << ";\n";
    break;
  case Kind::VarDef:
    out.printIndent();
    prog.getTypes().get(type).print(state);
    out << " ";
    out << prog.getIdents().getName(id);
    out << " = ";
    children.front().print(state);
    out << ";\n";
    break;
  case Kind::Asm:
    out.printIndent();
    out << "asm (\"" << constantValue << "\"::);\n";
    break;
  case Kind::StmtExpr:
    out.printIndent();
    printSummary(state);
    children.begin()->print(state);
    out << ";\n";
    break;
  case Kind::If: {
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "if");
    out << " (";
    children.at(0).print(state);
    out << ")\n";
    MaybeWrapInCurly wrap(out, children.at(1));
    IndentScope scope(out);
    children.at(1).print(state);
    break;
  }
  case Kind::IfElse: {
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "if");
    out << " (";
    children.at(0).print(state);
    out << ")\n";
    out.printIndent();
    out << "{\n";
    {
      IndentScope scope(out);
      children.at(1).print(state);
    }
    out.printIndent();
    out << "}\n";
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "else") << "\n";
    out.printIndent();
    out << "{\n";
    {
      IndentScope scope(out);
      children.at(2).print(state);
    }
    out.printIndent();
    out << "}\n";
    break;
  }
  case Kind::While: {
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "while");
    out << " (";
    children.at(0).print(state);
    out << ") \n";
    MaybeWrapInCurly wrap(out, children.at(1));
    children.at(1).print(state);
    out << "\n";
    break;
  }
  case Kind::Break:
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "break");
    out << ";\n";
    break;
  case Kind::Continue:
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "continue");
    out << ";\n";
    break;
  case Kind::Constant: {
    bool needsCast = prog.getBuiltin().literalNeedsCast(type);
    if (needsCast) {
      out << "((";
      prog.getTypes().get(type).print(state);
      out << ")";
    }
    out << constantValue;
    if (needsCast)
      out << ")";
    break;
  }
  case Kind::ArrayConstant: {
    out << "{";
    bool first = true;
    for (size_t i = 0; i < children.size(); ++i) {
      if (first)
        first = false;
      else
        out << ", ";
      children.at(i).print(state);
    }
    out << "}";
    break;
  }
  case Kind::IndirectCall: {
    out << "((/*icall*/*(";
    children.front().print(state);
    out << "))(";
    bool first = true;
    for (size_t i = 1; i < children.size(); ++i) {
      if (first)
        first = false;
      else
        out << ", ";
      children.at(i).print(state);
    }
    out << "))";
    break;
  }
  case Kind::Call: {
    out << prog.getIdents().getName(id) << "(";
    bool first = true;
    for (const Statement &u : children) {
      if (first)
        first = false;
      else
        out << ",";
      u.print(state);
    }
    out << ")";
    break;
  }
  case Kind::Return:
    out.printIndent();
    out.printColor(OutStream::Color::Magenta, "return ");
    printChildren(state);
    out << ";\n";
    break;
#define BINARY_OP_PRINT(NAME, SIGN)                                            \
  {                                                                            \
  case Kind::NAME:                                                             \
    printSummary(state);                                                       \
    out << "(";                                                                \
    children.at(0).print(state);                                               \
    out << SIGN;                                                               \
    children.at(1).print(state);                                               \
    out << ")";                                                                \
    break;                                                                     \
  }

    BINARY_OP_PRINT(Assign, "=")
    BINARY_OP_PRINT(Add, "+")
    BINARY_OP_PRINT(Sub, "-")
    BINARY_OP_PRINT(Div, "/")
    BINARY_OP_PRINT(Mul, "*")
    BINARY_OP_PRINT(Mod, "%")
    BINARY_OP_PRINT(And, "&&")
    BINARY_OP_PRINT(Or, "||")
    BINARY_OP_PRINT(Xor, "^")
    BINARY_OP_PRINT(BitwiseAnd, "&")
    BINARY_OP_PRINT(BitwiseOr, "|")
    BINARY_OP_PRINT(Equal, "==")
    BINARY_OP_PRINT(NotEqual, "!=")
    BINARY_OP_PRINT(Less, "<=")
    BINARY_OP_PRINT(LessEq, "<=")
    BINARY_OP_PRINT(Greater, ">")
    BINARY_OP_PRINT(GreaterEq, ">=")
    BINARY_OP_PRINT(ShiftLeft, "<<")
    BINARY_OP_PRINT(ShiftRight, ">>")

#undef BINARY_OP_PRINT
  }
}

void Statement::dumpStmt(const Program &p) const {
  OutString out(/*fancy=*/true);
  PrintState s(p, out);
  print(s);
  std::cerr << out.getStr() << "\n";
  if (getExtraData())
    getExtraData()->dump(p);
}

bool Statement::usesType(TypeRef t) const {
  if (this->type == t)
    return true;
  if (getExtraData() && getExtraData()->usesType(t))
    return true;
  for (const Statement &c : children)
    if (c.usesType(t))
      return true;
  return false;
}

void Statement::verifySelf(const Program &p) const {
  if (type != Void())
    SCCAssert(p.getTypes().isValid(type), "Must have a valid non-void type");
  for (const Statement &child : children) {
    if (*this == Statement::Kind::Compound) {
      SCCAssert(child.isStmt(), "Compound with expression as child?");
    }
    child.verifySelf(p);
  }
}

void Statement::printSummary(PrintState &state, bool printIndent) const {
  const ExtraData *d = getExtraData();
  if (!d)
    return;
  OutStream &out = state.getOut();
  const Program &prog = state.getProgram();
  std::string summary = d->getSummary(prog);
  if (summary.empty())
    return;

  if (printIndent)
    out.printIndent();

  out.printColor(OutStream::Color::BoldBlue, "/*");
  out.printColor(OutStream::Color::BoldBlue, d->getSummary(prog));
  out.printColor(OutStream::Color::BoldBlue, "*/");

  if (printIndent)
    state.getOut() << "\n";
}

void Statement::printChildren(PrintState &state) const {
  for (const Statement &u : children)
    u.print(state);
}

ExtraData::~ExtraData() {}
