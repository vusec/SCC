#pragma once

#include "scc/program/IdentTable.h"
#include "scc/program/PrintState.h"
#include "scc/program/Variable.h"
#include "scc/utils/CopyableUniquePtr.h"
#include "scc/utils/OutStream.h"

#include <functional>
#include <memory>
#include <set>
#include <vector>

class Program;
class Statement;

/// Extra data associated with statements.
struct ExtraData {
  virtual ~ExtraData();
  virtual std::unique_ptr<ExtraData> clone() const = 0;
  virtual void dump(const Program &p) const = 0;
  virtual std::string getSummary(const Program &p) const = 0;
  virtual void printPrefix(const Statement &s, PrintState &state) const {}
  virtual void printSuffix(const Statement &s, PrintState &state) const {}
  virtual bool usesType(TypeRef t) const { return false; }
  virtual bool usesID(NameID id) const { return false; }
};

/// Describes a statement/expression in a C/C++ AST.
class Statement {
  static void expectExpr(const Statement &s) {
    assert(s.isExpr());
    assert(s.getKind() != Kind::Compound);
  }
  void verify() const;

public:
  static constexpr size_t maxChildren = 64000;

  enum class Kind {
    Compound,
    Return,
    If,
    IfElse,
    While,
    Empty,
    Break,
    Continue,
    Assign,
    Subscript,
    Add,
    Sub,
    Div,
    Mul,
    Mod,
    NotEqual,
    Equal,
    Less,
    LessEq,
    Greater,
    GreaterEq,
    ShiftLeft,
    ShiftRight,
    BitwiseAnd,
    BitwiseOr,
    And,
    Or,
    Xor,
    Not,
    StmtExpr,
    AttrStmt,
    AddrOf,
    AddrOfFunc,
    Deref,
    Asm,
    Cast,
    IndirectCall,
    Call,
    GotoLabel,
    Goto,
    New,
    Delete,
    Throw,
    Try,
    Catch,
    VarDecl,
    VarDef,
    GlobalVarRef,
    LocalVarRef,
    Constant,
    Arrow,
    Dot,
    Sizeof,
    ArrayConstant,
    Group,
  };

  /// Creates any constant expression. The string is the
  /// actual string emitted in the final program.
  [[nodiscard]] static Statement Constant(std::string val, TypeRef t) {
    Statement res(Kind::Constant);
    res.constantValue = val;
    res.type = t;
    return res;
  }

  /// Creates any constant array expression.
  [[nodiscard]] static Statement ConstantArray(std::vector<Statement> c,
                                               TypeRef t) {
    Statement res(Kind::ArrayConstant);
    res.children = c;
    res.type = t;
    return res;
  }

  [[nodiscard]] static const std::vector<Kind> getPtrArithmeticOps() {
    return {Kind::Add, Kind::Sub};
  }

  [[nodiscard]] static const std::vector<Kind> getPtrPtrOps() {
    return {
        Kind::NotEqual, Kind::Equal,     Kind::Less,      Kind::LessEq,
        Kind::Greater,  Kind::GreaterEq, Kind::ShiftLeft,
    };
  }

  [[nodiscard]] static const std::vector<Kind> getIntArithmeticOps() {
    return {
        Kind::Assign,    Kind::Add,        Kind::Sub,        Kind::Div,
        Kind::Mul,       Kind::Mod,        Kind::BitwiseAnd, Kind::BitwiseOr,
        Kind::And,       Kind::Or,         Kind::Equal,      Kind::NotEqual,
        Kind::Less,      Kind::LessEq,     Kind::Greater,    Kind::GreaterEq,
        Kind::ShiftLeft, Kind::ShiftRight,
    };
  }

  [[nodiscard]] static const std::vector<Kind> getFloatArithmeticOps() {
    return {Kind::Assign, Kind::Add,        Kind::Sub,       Kind::Div,
            Kind::Mul,    Kind::BitwiseAnd, Kind::BitwiseOr, Kind::And,
            Kind::Or,     Kind::NotEqual,   Kind::Equal,     Kind::Less,
            Kind::LessEq, Kind::Greater,    Kind::GreaterEq};
  }

  [[nodiscard]] static const std::vector<Kind> getBinaryOps() {
    return {
        Kind::Assign,    Kind::Add,        Kind::Sub,        Kind::Div,
        Kind::Mul,       Kind::Mod,        Kind::BitwiseAnd, Kind::BitwiseOr,
        Kind::And,       Kind::Or,         Kind::NotEqual,   Kind::Equal,
        Kind::Less,      Kind::LessEq,     Kind::Greater,    Kind::GreaterEq,
        Kind::ShiftLeft, Kind::ShiftRight,
    };
  }

  [[nodiscard]] static const std::set<Kind> getBinaryOpsSet() {
    return {
        Kind::Assign,    Kind::Add,        Kind::Sub,        Kind::Div,
        Kind::Mul,       Kind::Mod,        Kind::BitwiseAnd, Kind::BitwiseOr,
        Kind::And,       Kind::Or,         Kind::NotEqual,   Kind::Equal,
        Kind::Less,      Kind::LessEq,     Kind::Greater,    Kind::GreaterEq,
        Kind::ShiftLeft, Kind::ShiftRight,
    };
  }

  [[nodiscard]] static const std::set<Kind> getComparisonOps() {
    return {Kind::And,  Kind::Or,     Kind::NotEqual, Kind::Equal,
            Kind::Less, Kind::LessEq, Kind::Greater,  Kind::GreaterEq};
  }

  /// Create a binary operator.
  [[nodiscard]] static Statement BinaryOp(Program &p, Kind op, Statement lhs,
                                          Statement rhs);

  [[nodiscard]] static Statement Assign(Program &p, Statement lhs,
                                        Statement rhs) {
    return BinaryOp(p, Kind::Assign, lhs, rhs);
  }

  /// Create a compound assignment (e.g., +=, -=, %=, ...).
  [[nodiscard]] static Statement CompoundAssign(Program &p, Kind op,
                                                Statement lhs, Statement rhs) {
    return Assign(p, lhs, BinaryOp(p, op, lhs, rhs));
  }

  /// Create a `break` statement.
  [[nodiscard]] static Statement Break() { return Statement(Kind::Break); }

  /// Create a `continue` statement.
  [[nodiscard]] static Statement Continue() {
    return Statement(Kind::Continue);
  }

  /// Create a `&value` expression on the given expression.
  [[nodiscard]] static Statement AddrOf(TypeRef t, Statement expr) {
    Statement res(Kind::AddrOf, {expr});
    res.type = t;
    assert(t != Void());
    return res;
  }

  /// Create a `&value` expression on the given expression.
  [[nodiscard]] static Statement AddrOfFunc(TypeRef t, NameID funcName) {
    Statement res(Kind::AddrOfFunc);
    res.id = funcName;
    res.type = t;
    assert(t != Void());
    return res;
  }

  /// Creates a `*value` expression on the given expression.
  [[nodiscard]] static Statement Deref(TypeRef t, Statement expr) {
    Statement res(Kind::Deref, {expr});
    res.type = t;
    return res;
  }

  /// Create a `base->member` expression.
  [[nodiscard]] static Statement Arrow(TypeRef t, Statement base,
                                       NameID member) {
    Statement res(Kind::Arrow, {base});
    res.type = t;
    res.id = member;
    assert(t != Void());
    return res;
  }

  /// Create a `base.member` expression.
  [[nodiscard]] static Statement Dot(TypeRef t, Statement base, NameID member) {
    Statement res(Kind::Dot, {base});
    res.type = t;
    res.id = member;
    assert(t != Void());
    return res;
  }

  [[nodiscard]] static Statement Asm(std::string assembly) {
    Statement res(Kind::Asm);
    res.constantValue = assembly;
    return res;
  }

  /// Wraps a plain expression in a statement. E.g.
  /// the expression `0` needs to be wrapped to
  /// be a statement (that gets a trailing semicolon).
  [[nodiscard]] static Statement StmtExpr(Statement expr) {
    return Statement(Kind::StmtExpr, {expr});
  }

  /// An empty statement (just `;` in the final code).
  [[nodiscard]] static Statement Empty() { return Statement(Kind::Empty); }
  [[nodiscard]] static Statement CommentStmt(std::string comment) {
    auto res = Empty();
    res.constantValue = comment;
    return res;
  }

  /// An if statement with a condition.
  [[nodiscard]] static Statement If(Statement cond, Statement body) {
    return Statement(Kind::If, {cond, ensureCompound(body)});
  }

  /// An if statement with a condition.
  [[nodiscard]] static Statement IfElse(Statement cond, Statement body,
                                        Statement elseBody) {
    return Statement(Kind::IfElse,
                     {cond, ensureCompound(body), ensureCompound(elseBody)});
  }

  [[nodiscard]] static Statement AttrStmt(Statement child, std::string attr) {
    Statement res(Kind::AttrStmt, {child});
    res.constantValue = attr;
    return res;
  }

  /// A while loop.
  [[nodiscard]] static Statement While(Statement cond, Statement body) {
    return Statement(Kind::While, {cond, ensureCompound(body)});
  }

  /// A compount statement, (e.g. the curly braces
  /// around `{ some_other_statement; }`.
  [[nodiscard]] static Statement
  CompoundStmt(const std::vector<Statement> &children) {
    assert(children.size() < maxChildren);
    for (const Statement &child : children) {
      SCCAssert(child.isStmt(), "Passed expr to compound statement?");
    }
    return Statement(Kind::Compound, children);
  }

  /// A 'group' of statements that doesn't have a scope.
  /// Doesn't match to a specific syntax.
  [[nodiscard]] static Statement Group(const std::vector<Statement> &children,
                                       std::string comment = "") {
    SCCAssert(children.size() < maxChildren, "Too many children in group");
    for (const Statement &child : children)
      SCCAssert(child.isStmt(), "Only statements allowed in a group");
    Statement res(Kind::Group, children);
    res.constantValue = comment;
    return res;
  }

  /// `return;`
  [[nodiscard]] static Statement VoidReturn() {
    return Statement(Kind::Return);
  }

  /// Indirect call.
  [[nodiscard]] static Statement IndirectCall(TypeRef ret, Statement funcPtr,
                                              std::vector<Statement> args) {
    args.insert(args.begin(), funcPtr);
    Statement res(Kind::IndirectCall, args);
    res.type = ret;
    return res;
  }

  /// A function call.
  [[nodiscard]] static Statement Call(TypeRef ret, NameID funcName,
                                      const std::vector<Statement> &children) {
    Statement res(Kind::Call, children);
    res.id = funcName;
    res.type = ret;
    res.verify();
    return res;
  }

  /// A goto label.
  [[nodiscard]] static Statement GotoLabel(NameID labelName) {
    Statement res(Kind::GotoLabel);
    res.id = labelName;
    return res;
  }

  /// A goto statement.
  [[nodiscard]] static Statement Goto(NameID labelName) {
    Statement res(Kind::Goto);
    res.id = labelName;
    return res;
  }

  [[nodiscard]] static Statement Subscript(TypeRef t, Statement base,
                                           Statement index) {
    expectExpr(base);
    expectExpr(index);

    Statement res(Kind::Subscript, {base, index});
    res.type = t;
    return res;
  }

  /// A C-style cast.
  [[nodiscard]] static Statement Cast(TypeRef toType, Statement expr) {
    Statement res(Kind::Cast, {expr});
    res.type = toType;
    return res;
  }

  /// A new statement in C++.
  [[nodiscard]] static Statement New(TypeRef ptrType,
                                     std::vector<Statement> args) {
    Statement res(Kind::New, args);
    res.type = ptrType;
    return res;
  }

  /// A delete statement in C++.
  [[nodiscard]] static Statement Delete(Statement expr) {
    Statement res(Kind::Delete, {expr});
    return res;
  }

  /// A throw statement in C++.
  [[nodiscard]] static Statement Throw(Statement expr) {
    Statement res(Kind::Throw, {expr});
    return res;
  }

  /// A try statement in C++.
  [[nodiscard]] static Statement Try(Statement expr,
                                     std::vector<Statement> catches) {
    catches.insert(catches.begin(), expr);
    Statement res(Kind::Try, catches);
    return res;
  }

  /// A catch clause in C++ (has to be nested in a Try Statement).
  [[nodiscard]] static Statement Catch(TypeRef caughtT, NameID caughtId,
                                       Statement body) {
    Statement res(Kind::Catch, {body});
    res.type = caughtT;
    res.id = caughtId;
    return res;
  }

  [[nodiscard]] static Statement CatchAll(Statement body) {
    Statement res(Kind::Catch, {body});
    return res;
  }

  /// A return with a value.
  [[nodiscard]] static Statement Return(Statement val) {
    return Statement(Kind::Return, {val});
  }

  /// A sizeof(type) expression.
  [[nodiscard]] static Statement SizeOf(Program &p, TypeRef type);

  /// A reference to a global variable.
  [[nodiscard]] static Statement GlobalVarRef(Variable var);

  /// A reference to a local variable.
  [[nodiscard]] static Statement LocalVarRef(Variable v) {
    Statement res(Kind::LocalVarRef);
    res.id = v.getName();
    res.type = v.getType();
    return res;
  }

  /// A statement that declares (but not defines) a variable.
  /// E.g. `int i;`
  [[nodiscard]] static Statement VarDecl(TypeRef t, NameID name) {
    Statement res(Kind::VarDecl);
    res.id = name;
    res.type = t;
    return res;
  }

  [[nodiscard]] Statement getVarRef() const {
    assert(kind == Kind::VarDecl || kind == Kind::VarDef);
    return LocalVarRef(getReferencedVarInfo());
  }

  [[nodiscard]] Variable getReferencedVarInfo() const {
    assert(kind == Kind::VarDecl || kind == Kind::VarDef ||
           kind == Kind::LocalVarRef || kind == Kind::GlobalVarRef);
    return Variable(type, id);
  }

  /// A statement that declares and defines a variable.
  /// E.g. `int i = 3;`
  [[nodiscard]] static Statement VarDef(TypeRef t, NameID name, Statement val) {
    Statement res(Kind::VarDef, {val});
    assert(val.type != Void());
    res.id = name;
    res.type = t;
    return res;
  }

  /// Whether this is just a semicolon.
  [[nodiscard]] bool isEmpty() const { return kind == Kind::Empty; }

  /// Prints the C code for this statement (and its children).
  void print(PrintState &state) const;

  void dumpStmt(const Program &p) const;

  // foreach loop support.
  auto begin() { return children.begin(); }
  auto end() { return children.end(); }
  auto begin() const { return children.begin(); }
  auto end() const { return children.end(); }
  const auto &getChildren() const { return children; }

  size_t getNumChildren() const { return children.size(); }

  NameID getJumpTarget() const {
    switch (kind) {
    case Kind::GotoLabel:
      return id;
    default:
      assert(false && "Not a variable?");
      return InvalidName;
    }
  }

  NameID getGotoLabelID() const {
    switch (kind) {
    case Kind::Goto:
      return id;
    default:
      assert(false && "Not a goto label?");
      return InvalidName;
    }
  }

  Statement &getChildWithIndex(size_t index) {
    assert(index < getNumChildren());
    return children.at(index);
  }

  TypeRef getVariableType() const {
    switch (kind) {
    case Kind::VarDecl:
    case Kind::VarDef:
    case Kind::Catch:
      return type;
    default:
      assert(false && "Not a variable?");
      return Void();
    }
  }

  TypeRef getEvalType() const {
    switch (kind) {
    case Kind::VarDecl:
    case Kind::VarDef:
      return Void();
    default:
      return type;
    }
  }

  Kind getKind() const { return kind; }

  Statement() {}

  ExtraData *getExtraData() { return extraData.data.get(); }
  const ExtraData *getExtraData() const { return extraData.data.get(); }

  void setExtraData(std::unique_ptr<ExtraData> &&d) {
    extraData.data = std::move(d);
  }
  void removeExtraData() { extraData.data.reset(); }

  NameID getDeclaredVarID() const {
    assert(kind == Kind::VarDecl || kind == Kind::VarDef ||
           kind == Kind::Catch);
    return id;
  }

  NameID getReferencedVarID() const {
    SCCAssert(kind == Kind::GlobalVarRef || kind == Kind::LocalVarRef,
              "Only variable refs can reference variables");
    return id;
  }

  NameID getCalledFuncID() const {
    SCCAssert(kind == Kind::Call, "Only calls have function ids.");
    return id;
  }

  bool usesType(TypeRef t) const;

  bool usesID(NameID wantedId) const {
    if (id == wantedId)
      return true;
    if (getExtraData() && getExtraData()->usesID(wantedId))
      return true;
    for (const Statement &c : *this)
      if (c.usesID(wantedId))
        return true;
    return false;
  }

  struct StmtAndParent {
    Statement *stmt = nullptr;
    Statement *parent = nullptr;
  };

  LoopCtrl
  foreachChild(const std::function<LoopCtrl(const Statement &)> &f) const {
    if (f(*this) == LoopCtrl::Abort)
      return LoopCtrl::Abort;
    for (const Statement &child : *this)
      if (child.foreachChild(f) == LoopCtrl::Abort)
        return LoopCtrl::Abort;
    return LoopCtrl::Continue;
  }

  LoopCtrl modifyEachChild(const std::function<LoopCtrl(Statement &)> &f) {
    if (f(*this) == LoopCtrl::Abort)
      return LoopCtrl::Abort;
    for (Statement &child : *this)
      if (child.modifyEachChild(f) == LoopCtrl::Abort)
        return LoopCtrl::Abort;
    return LoopCtrl::Continue;
  }

  bool forAllChildren(const std::function<bool(const Statement &)> &f) const {
    if (!f(*this))
      return false;
    for (const Statement &child : *this)
      if (!child.forAllChildren(f))
        return false;
    return true;
  }

  std::vector<StmtAndParent> getAllChildren() {
    std::vector<StmtAndParent> res;
    getAllChildren(res);
    return res;
  }

  const std::vector<StmtAndParent> getAllChildren() const {
    return const_cast<Statement *>(this)->getAllChildren();
  }

  bool declaresVariable() const {
    return kind == Kind::VarDecl || kind == Kind::VarDef;
  }

  bool isStmt() const { return !isExpr(); }
  bool isExpr() const {
    switch (kind) {
    case Kind::Compound:
    case Kind::Return:
    case Kind::If:
    case Kind::IfElse:
    case Kind::Group:
    case Kind::While:
    case Kind::Empty:
    case Kind::Break:
    case Kind::Continue:
    case Kind::StmtExpr:
    case Kind::AttrStmt:
    case Kind::Asm:
    case Kind::GotoLabel:
    case Kind::Goto:
    case Kind::Throw:
    case Kind::Try:
    case Kind::Catch:
    case Kind::VarDecl:
    case Kind::VarDef:
    case Kind::Delete:
      return false;
    default:
      return true;
    }
  }

  size_t countNodes() const {
    size_t res = isEmpty() ? 1 : 2;
    size_t childMultiplier = isExpr() ? 2 : 1;
    for (const Statement &s : children)
      res += s.countNodes() * childMultiplier;
    return res;
  }

  void verifySelf(const Program &p) const;

  bool operator==(Kind k) const { return kind == k; }
  bool operator!=(Kind k) const { return kind != k; }

  std::vector<TypeRef> getReferencedTypes() const {
    std::vector<TypeRef> result;
    if (type != Void())
      result.push_back(type);
    if (otherType != Void())
      result.push_back(otherType);
    return result;
  }

private:
  static Statement ensureCompound(const Statement &s) {
    if (s == Kind::Group)
      return CompoundStmt({s});
    return s;
  }

  void getAllChildren(std::vector<StmtAndParent> &res) {
    for (Statement &c : *this) {
      StmtAndParent s;
      s.stmt = &c;
      s.parent = this;
      res.push_back(s);
      c.getAllChildren(res);
    }
  }

  void printSummaryLine(PrintState &state) const {
    printSummary(state, /*printIndent=*/true);
  }
  void printSummary(PrintState &state, bool printIndent = false) const;

  void printChildren(PrintState &state) const;

  Kind kind = Kind::Empty;
  std::vector<Statement> children;
  IdentTable::NameID id = InvalidName;
  std::string constantValue;
  TypeRef type = Void();
  TypeRef otherType = Void();
  CopyableUniquePtr<ExtraData> extraData;

  Statement(Kind k) : kind(k) {}
  Statement(Kind k, std::vector<Statement> children)
      : kind(k), children(children) {}
};

typedef std::unique_ptr<Statement> StatementUP;
