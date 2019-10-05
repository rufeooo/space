#include "gtest/gtest.h"

#include "mat.h"

TEST(MatrixTest, MatrixInitialization) {
  math::Mat4f mat4x4{
    1.0f, 0.0f, 2.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    3.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  ASSERT_EQ(mat4x4(0, 2), 3.0f);
  ASSERT_EQ(mat4x4(2, 0), 2.0f);
  math::Mat<int, 3, 6> mat3x6 {
    1, 2, 3,
    4, 5, 6,
    7, 8, 9,
    10, 11, 12,
    13, 14, 15,
    16, 17, 18
  };
  ASSERT_EQ(mat3x6(0, 0), 1);
  ASSERT_EQ(mat3x6(1, 0), 2);
  ASSERT_EQ(mat3x6(2, 0), 3);
  ASSERT_EQ(mat3x6(0, 3), 10);
  ASSERT_EQ(mat3x6(1, 3), 11);
  ASSERT_EQ(mat3x6(2, 3), 12);
  ASSERT_EQ(mat3x6(0, 5), 16);
  ASSERT_EQ(mat3x6(1, 5), 17);
  ASSERT_EQ(mat3x6(2, 5), 18);
}

TEST(MatrixTest, MatrixMultiplication) {
  math::Mat<int, 3, 2> mat3x2 {
    1, 2, 3,
    4, 5, 6
  };
  math::Mat<int, 2, 5> mat2x5 {
    1, 2,
    3, 4,
    5, 6,
    7, 8,
    9, 10
  };
  math::Mat<int, 3, 5> desired {
    9, 12, 15,
    19, 26, 33,
    29, 40, 51,
    39, 54, 69,
    49, 68, 87
  };
  auto r = mat3x2 * mat2x5;
  ASSERT_EQ(r.rows(), 3);
  ASSERT_EQ(r.cols(), 5);
  ASSERT_EQ(r, desired);
  math::Mat<int, 2, 2> mat2x2  { 1, 2, 3, 4 };
  math::Mat<int, 2, 2> desired2 { 2, 4, 6, 8 };
  ASSERT_EQ(mat2x2 * 2, desired2);
  ASSERT_EQ(2 * mat2x2, desired2);
}

TEST(MatrixTest, MatrixDivision) {
  math::Mat<int, 2, 2> mat2x2  { 2, 4, 6, 8 };
  math::Mat<int, 2, 2> desired { 1, 2, 3, 4 };
  ASSERT_EQ(mat2x2 / 2, desired);
}

TEST(MatrixTest, MatrixAddition) {
  math::Mat<int, 2, 2> mat2x2  { 1, 2, 3, 4 };
  math::Mat<int, 2, 2> desired { 4, 5, 6, 7 };
  ASSERT_EQ(mat2x2 + 3, desired);
  ASSERT_EQ(3 + mat2x2, desired);
}

TEST(MatrixTest, MatrixSubtraction) {
  math::Mat<int, 2, 2> mat2x2  { 1, 2, 3, 4 };
  math::Mat<int, 2, 2> desired { -1, 0, 1, 2 };
  ASSERT_EQ(mat2x2 - 2, desired);
}

TEST(MatrixTest, MatrixVectorMultiplyTranslate) {
  math::Mat4f translate {
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    1.f, 2.f, 3.f, 1.f,
  };
  math::Vec3f point(0.f, 0.f, 0.f);
  math::Vec3f result = translate * point;
  ASSERT_EQ(result, math::Vec3f(1.f, 2.f, 3.f));
}

TEST(MatrixTest, MatrixVectorMultiplyScale) {
  math::Mat4f scale {
    1.f, 0.f, 0.f, 0.f,
    0.f, 2.f, 0.f, 0.f,
    0.f, 0.f, 3.f, 0.f,
    0.f, 0.f, 0.f, 1.f,
  };
  math::Vec3f point(1.f, 1.f, 1.f);
  math::Vec3f result = scale * point;
  ASSERT_EQ(result, math::Vec3f(1.f, 2.f, 3.f));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
