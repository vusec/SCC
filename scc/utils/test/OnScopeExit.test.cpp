#include "scc/utils/OnScopeExit.h"
#include "gtest/gtest.h"

TEST(OnScopeExit, Basic) {
  int dtor = 0;
  {
    OnScopeExit s([&]() { dtor += 1; });
    ASSERT_EQ(dtor, 0);
  }
  ASSERT_EQ(dtor, 1);
}
