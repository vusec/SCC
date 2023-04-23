#include "scc/program/BuiltinFunctions.h"
#include "gtest/gtest.h"
#include <iostream>

using BF = BuiltinFunctions;
using Kind = BF::Kind;

TEST(TestBuiltinFunctions, getKindForID) {
  EXPECT_EQ(BuiltinFunctions::getKindForID("memcpy"), Kind::MemCpy);
  EXPECT_EQ(BuiltinFunctions::getKindForID("strcpy"), Kind::StrCpy);
  EXPECT_EQ(BuiltinFunctions::getKindForID("malloc"), Kind::Malloc);
  EXPECT_EQ(BuiltinFunctions::getKindForID("free"), Kind::Free);

  EXPECT_FALSE(BuiltinFunctions::getKindForID(""));
  EXPECT_FALSE(BuiltinFunctions::getKindForID(" "));
  EXPECT_FALSE(BuiltinFunctions::getKindForID("free "));
  EXPECT_FALSE(BuiltinFunctions::getKindForID("asd"));
}
