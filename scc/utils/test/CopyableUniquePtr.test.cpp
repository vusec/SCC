#include "scc/utils/CopyableUniquePtr.h"
#include "gtest/gtest.h"

struct DummyClonable {
  int value = 0;
  DummyClonable(int val) : value(val) {}
  virtual std::unique_ptr<DummyClonable> clone() const {
    return std::make_unique<DummyClonable>(*this);
  }
};

TEST(CopyableUniquePtr, Basic) {
  CopyableUniquePtr<DummyClonable> ptr;
  ptr.data.reset(new DummyClonable(4));
  ASSERT_EQ(ptr->value, 4);
  auto copy = ptr;
  ASSERT_EQ(ptr->value, 4);
  ASSERT_EQ(copy->value, 4);
  // Check that modifying the original doesn't change the copy.
  ptr->value = 5;
  ASSERT_EQ(ptr->value, 5);
  ASSERT_EQ(copy->value, 4);
}
