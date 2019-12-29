#include "gtest/gtest.h"

#include "mat.h"
#include "mat_ops.h"
#include "vec.h"

TEST(MatrixOperationsTest, CreateIdentityMatrix)
{
  math::Mat4f identity_matrix_4 = math::CreateIdentityMatrix<float, 4>();
  math::Mat4f desired{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  ASSERT_EQ(identity_matrix_4, desired);
}

TEST(MatrixOperationsTest, CreateTranslationMatrix)
{
  math::Mat4f translation_matrix =
      math::CreateTranslationMatrix(math::Vec3f(1.0f, 2.0f, 3.0f));
  math::Mat4f desired{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 1.0f};
  ASSERT_EQ(translation_matrix, desired);
}

int
main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
