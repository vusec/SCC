#include "scc/utils/OutStream.h"
#include "gtest/gtest.h"

TEST(OutString, Basic) {
  OutString s;
  s << "foo";
  s << "bar";
  ASSERT_EQ(s.getStr(), "foobar");
}
