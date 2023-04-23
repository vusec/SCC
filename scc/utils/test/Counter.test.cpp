#include "scc/utils/Counter.h"
#include "gtest/gtest.h"

TEST(CounterTest, Basic) {
  unsigned val = 0;
  for (auto c : count::upTo(100)) {
    ++val;
  }
  EXPECT_EQ(val, 100U);
}

TEST(CounterTest, Empty) {
  for (auto c : count::upTo(0))
    FAIL() << "Shouldn't have counted with 0";
}
