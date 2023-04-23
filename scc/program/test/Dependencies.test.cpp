#include "scc/program/Dependencies.h"
#include "scc/program/GlobalVar.h"
#include "scc/program/Program.h"
#include "scc/program/RecordDecl.h"
#include "gtest/gtest.h"
#include <iostream>

TEST(TestSourceDependencies, GlobalVarWithIntArray) {
  Program p;
  SourceDependencies deps;
  TypeRef array = p.getTypes().addType(
      Type::Array(p.getBuiltin().signed_int, 20, p.getIdents().makeNewID()));
  GlobalVar &var =
      p.add(std::make_unique<GlobalVar>(array, p.getIdents().makeNewID()));
  deps.add(SourceDependency(var));
  deps.getOrdered(p);

  const auto depList = deps.getOrdered(p);
  EXPECT_EQ(depList->size(), 3U);

  EXPECT_EQ(depList->at(0), SourceDependency(p.getBuiltin().signed_int));
  // We first have to declare the array type.
  EXPECT_EQ(depList->at(1), SourceDependency(array));
  // Then we declare the variable.
  EXPECT_EQ(depList->at(2), SourceDependency(var));
}

TEST(TestSourceDependencies, FunctionWithIntArray) {
  Program p;
  SourceDependencies deps;
  TypeRef array = p.getTypes().addType(
      Type::Array(p.getBuiltin().signed_int, 20, p.getIdents().makeNewID()));
  std::vector<Variable> args;
  Function &f = p.add(std::make_unique<Function>(
      p.getBuiltin().signed_int, p.getIdents().makeNewID(), args));
  f.setBody(Statement::CompoundStmt(
      {Statement::VarDecl(array, p.getIdents().makeNewID())}));

  deps.add(SourceDependency(f));

  const auto depList = deps.getOrdered(p);
  EXPECT_EQ(depList->size(), 3U);

  EXPECT_EQ(depList->at(0), SourceDependency(p.getBuiltin().signed_int));
  // We first have to declare the array type.
  EXPECT_EQ(depList->at(1), SourceDependency(array));
  // Then we declare the variable.
  EXPECT_EQ(depList->at(2), SourceDependency(f));
}

TEST(TestSourceDependencies, FunctionWithGlobalRef) {
  Program p;
  SourceDependencies deps;

  GlobalVar &var = p.add(std::make_unique<GlobalVar>(
      p.getBuiltin().signed_int, p.getIdents().makeNewID()));

  std::vector<Variable> args;
  Function &f = p.add(std::make_unique<Function>(
      p.getBuiltin().signed_int, p.getIdents().makeNewID(), args));
  f.setBody(Statement::CompoundStmt(
      {Statement::StmtExpr(Statement::GlobalVarRef(var.getAsVar()))}));

  deps.add(SourceDependency(var));
  deps.add(SourceDependency(f));

  const auto depList = deps.getOrdered(p);
  EXPECT_EQ(depList->size(), 3U);

  EXPECT_EQ(depList->at(0), SourceDependency(p.getBuiltin().signed_int));
  // We first have to declare the variable.
  EXPECT_EQ(depList->at(1), SourceDependency(var));
  // Then we emit the function using the variable.
  EXPECT_EQ(depList->at(2), SourceDependency(f));
}

TEST(TestSourceDependencies, FunctionCallingFunction) {
  Program p;
  SourceDependencies deps;

  std::vector<Variable> args;
  Function &f1 = p.add(std::make_unique<Function>(
      p.getBuiltin().signed_int, p.getIdents().makeNewID(), args));
  f1.setBody(Statement::CompoundStmt({}));

  Function &f2 = p.add(std::make_unique<Function>(
      p.getBuiltin().signed_int, p.getIdents().makeNewID(), args));
  f2.setBody(Statement::CompoundStmt({Statement::StmtExpr(
      Statement::Call(f1.getReturnType(), f1.getNameID(), {}))}));

  deps.add(SourceDependency(f1));
  deps.add(SourceDependency(f2));

  const auto depList = deps.getOrdered(p);
  EXPECT_EQ(depList->size(), 3U);

  EXPECT_EQ(depList->at(0), SourceDependency(p.getBuiltin().signed_int));
  // We first emit the function that is being called.
  EXPECT_EQ(depList->at(1), SourceDependency(f1));
  // Then the function calling the other function.
  EXPECT_EQ(depList->at(2), SourceDependency(f2));
}

TEST(TestSourceDependencies, GlobalVarWithStructArray) {
  Program p;
  SourceDependencies deps;

  Record &rec = p.add(Record::Struct(p, p.getIdents().makeNewID(), {}));
  TypeRef array = p.getTypes().addType(
      Type::Array(rec.getType(), 20, p.getIdents().makeNewID()));
  GlobalVar &var =
      p.add(std::make_unique<GlobalVar>(array, p.getIdents().makeNewID()));

  deps.add(SourceDependency(rec));
  deps.add(SourceDependency(var));
  deps.add(SourceDependency(rec));

  const auto depList = deps.getOrdered(p);
  ASSERT_EQ(depList->size(), 4U);

  // First we declare the record and make sure the type exists.
  EXPECT_EQ(depList->at(0), SourceDependency(rec));
  EXPECT_EQ(depList->at(1), SourceDependency(rec.getType()));
  // Then we declare the array of records.
  EXPECT_EQ(depList->at(2), SourceDependency(array));
  // Finally we can emit the variable.
  EXPECT_EQ(depList->at(3), SourceDependency(var));
}
