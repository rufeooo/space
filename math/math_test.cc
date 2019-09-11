#define CATCH_CONFIG_MAIN

#include <iostream>
#include <catch2/catch.hpp>

#include "vec.h"

TEST_CASE("Add Vectors", "[vec]") {
  math::Vec2i a(3, 4);
  math::Vec2i b(9, 2);
  REQUIRE(a.x() == 3);
  REQUIRE(a.y() == 4);
  REQUIRE(b.x() == 9);
  REQUIRE(b.y() == 2);
  auto c = a + b;
  REQUIRE(c.x() == 12);
  REQUIRE(c.y() == 6);
  a += b;
  REQUIRE(a.x() == 12);
  REQUIRE(a.y() == 6);
  math::Vec3i aa(3, 4, 9);
  math::Vec3i bb(9, 2, 5);
  REQUIRE(aa.x() == 3);
  REQUIRE(aa.y() == 4);
  REQUIRE(aa.z() == 9);
  REQUIRE(bb.x() == 9);
  REQUIRE(bb.y() == 2);
  REQUIRE(bb.z() == 5);
  auto cc = aa + bb;
  REQUIRE(cc.x() == 12);
  REQUIRE(cc.y() == 6);
  REQUIRE(cc.z() == 14);
  aa += bb;
  REQUIRE(aa.x() == 12);
  REQUIRE(aa.y() == 6);
  REQUIRE(aa.z() == 14);

}

TEST_CASE("Subtract Vectors", "[vec]") {
  math::Vec2i a(3, 4);
  math::Vec2i b(9, 2);
  auto c = a - b;
  REQUIRE(c.x() == -6);
  REQUIRE(c.y() == 2);
  a -= b;
  REQUIRE(a.x() == -6);
  REQUIRE(a.y() == 2);
}

TEST_CASE("Multiply with scalar", "[vec]") {
  math::Vec2i a(3, 4);
  int b = 3;
  auto c = a * b;
  REQUIRE(c.x() == 9);
  REQUIRE(c.y() == 12);
  a *= b;
  REQUIRE(a.x() == 9);
  REQUIRE(a.y() == 12);
}

TEST_CASE("Divide with scalar", "[vec]") {
  math::Vec2i a(9, 6);
  int b = 3;
  auto c = a / b;
  REQUIRE(c.x() == 3);
  REQUIRE(c.y() == 2);
  a /= b;
  REQUIRE(a.x() == 3);
  REQUIRE(a.y() == 2);
}

TEST_CASE("Dot product", "[vec]") {
  math::Vec2i a(3, 4);
  math::Vec2i b(9, 2);
  REQUIRE(math::Dot(a, b) == 35);
  REQUIRE(a.Dot(b) == 35);
}

TEST_CASE("Normalize vector", "[vec]") {
  math::Vec2f a(3.0f, 4.0f);
  a.Normalize();
  REQUIRE(a.x() == 0.6f);
  REQUIRE(a.y() == 0.8f);
  REQUIRE(math::Length(a) == 1.0f);
}

TEST_CASE("Length / Length Squared", "[vec]") {
  math::Vec2i a(3, 4);
  REQUIRE(math::LengthSquared(a) == 25.0f);
}

TEST_CASE("Default Initialization", "[vec]") {
  math::Vec2i a;
  math::Vec2f b;
  math::Vec2d c;
  REQUIRE(a.x() == 0);
  REQUIRE(a.y() == 0);
  REQUIRE(b.x() == 0.0f);
  REQUIRE(b.y() == 0.0f);
  REQUIRE(c.x() == 0.0);
  REQUIRE(c.y() == 0.0);
}
