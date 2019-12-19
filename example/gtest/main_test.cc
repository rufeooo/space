#include "gtest/gtest.h"

int
Foo(int n)
{
  if (n == 0) return 1;
  return 0;
}

TEST(Foo, Negative) { EXPECT_EQ(Foo(-1), 0); }

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
