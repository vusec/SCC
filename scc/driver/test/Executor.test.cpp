#include "scc/driver/Executor.h"
#include "gtest/gtest.h"
#include <iostream>

const static std::optional<std::string> None = {};

TEST(TestExecutor, TestGetValue) {
  EXPECT_EQ(Executor::getValue("KEY:A", "KEY:"), "A");
  EXPECT_EQ(Executor::getValue("KEY:A\n", "KEY:"), "A");
  EXPECT_EQ(Executor::getValue("\nKEY:A\n\n", "KEY:"), "A");
  EXPECT_EQ(Executor::getValue("\nKEY:\nA\n\n", "KEY:"), "");
  EXPECT_EQ(Executor::getValue("", "KEY:"), None);
  EXPECT_EQ(Executor::getValue("NOTKEY:A\n", "KEY:"), None);
}

TEST(TestExecutor, TestHasValue) {
  EXPECT_EQ(Executor::hasValue("KEYA", "KEY"), false);
  EXPECT_EQ(Executor::hasValue("AKEY", "KEY"), false);
  EXPECT_EQ(Executor::hasValue("KEY", "KEY"), true);
  EXPECT_EQ(Executor::hasValue("a\nKEY", "KEY"), true);
  EXPECT_EQ(Executor::hasValue("a\nKEY\nb", "KEY"), true);
  EXPECT_EQ(Executor::hasValue("a\naKEY\nb", "KEY"), false);
  EXPECT_EQ(Executor::hasValue("a\nKEYb\nb", "KEY"), false);
}
