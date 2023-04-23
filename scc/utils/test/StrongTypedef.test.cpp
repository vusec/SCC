#include "scc/utils/StrongTypedef.h"
#include "gtest/gtest.h"

struct DummyTypedef : StrongTypedef<DummyTypedef, int> {};

TEST(StrongTypedef, Basic) {
  DummyTypedef d = DummyTypedef::fromInternalValue(1);
  ASSERT_EQ(d.getInternalVal(), 1);
}

TEST(StrongTypedef, Less) {
  DummyTypedef d1 = DummyTypedef::fromInternalValue(1);
  DummyTypedef d2 = DummyTypedef::fromInternalValue(2);
  ASSERT_TRUE(d1 < d2);
  ASSERT_FALSE(d2 < d1);
  ASSERT_FALSE(d1 < d1);
}

TEST(StrongTypedef, Equal) {
  DummyTypedef d1 = DummyTypedef::fromInternalValue(1);
  DummyTypedef d2 = DummyTypedef::fromInternalValue(2);
  ASSERT_TRUE(d1 == d1);
  ASSERT_TRUE(d2 == d2);
  ASSERT_FALSE(d1 == d2);
}

TEST(StrongTypedef, NotEqual) {
  DummyTypedef d1 = DummyTypedef::fromInternalValue(1);
  DummyTypedef d2 = DummyTypedef::fromInternalValue(2);
  ASSERT_FALSE(d1 != d1);
  ASSERT_FALSE(d2 != d2);
  ASSERT_TRUE(d1 != d2);
}
