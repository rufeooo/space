#include "gtest/gtest.h"

#include "intersection.h"
#include "vec.h"

TEST(Intersection, LineSegmentsIntersectPositive_One) {
  math::Vec2f a_start(0.f, 1.f), a_end(1.f, 1.f);
  math::Vec2f b_start(0.5f, 0.f), b_end(0.5f, 1.5f);
  ASSERT_TRUE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

TEST(Intersection, LineSegmentsIntersectPositive_Two) {
  math::Vec2f a_start(-1.f, -1.5f), a_end(-3.f, -5.f);
  math::Vec2f b_start(-5.f, 0.f), b_end(0.f, -5.f);
  ASSERT_TRUE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

TEST(Intersection, LineSegmentsIntersectPositive_Three) {
  math::Vec2f a_start(3.f, -5.f), a_end(0.f, 0.f);
  math::Vec2f b_start(0.f, -4.f), b_end(3.f, 0.f);
  ASSERT_TRUE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

TEST(Intersection, LineSegmentsIntersectNegative_One) {
  math::Vec2f a_start(0.f, 1.f), a_end(1.f, 1.f);
  math::Vec2f b_start(0.5f, 0.f), b_end(0.5f, -1.5f);
  ASSERT_FALSE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

TEST(Intersection, LineSegmentsIntersectNegative_Two) {
  math::Vec2f a_start(-1.f, -1.5f), a_end(-3.f, -5.f);
  math::Vec2f b_start(-5.f, 0.f), b_end(0.f, 5.f);
  ASSERT_FALSE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

TEST(Intersection, LineSegmentsIntersectNegative_Three) {
  math::Vec2f a_start(3.f, -5.f), a_end(0.f, 0.f);
  math::Vec2f b_start(0.f, -4.f), b_end(-3.f, 0.f);
  ASSERT_FALSE(math::LineSegmentsIntersect2D(
      a_start, a_end, b_start, b_end));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
