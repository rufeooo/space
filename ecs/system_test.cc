#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ecs.h"
#include "system.h"

struct PositionComponent {
  PositionComponent() = default;
  PositionComponent(int x, int y) : x_(x), y_(y) {}

  int x_;
  int y_;
};

struct VelocityComponent {
  VelocityComponent() = default;
  VelocityComponent(float dx, float dy) : dx_(dx), dy_(dy) {}

  float dx_;
  float dy_;
};

class PhysicsSystem
    : public ecs::System<PositionComponent, VelocityComponent> {
 public:
  void OnEntity(
      ecs::Entity entity,
      PositionComponent& position,
      VelocityComponent& velocity) override {
    position.x_ *= 2;
    position.y_ *= 2;
    velocity.dx_ *= 2.0f;
    velocity.dy_ *= 2.0f;
  }
};

TEST(SystemTest, HappyPathTest) {
  ecs::Clear<PositionComponent>(); ecs::Clear<VelocityComponent>();
  // Runs for entity 1.
  ecs::Assign<PositionComponent>(1, 10, 15);
  ecs::Assign<VelocityComponent>(1, 1.0f, 3.0f);
  // Does not run for entity 2.
  ecs::Assign<PositionComponent>(2, 10, 15);
  // Runs for entity 3.
  ecs::Assign<PositionComponent>(3, 5, 5);
  ecs::Assign<VelocityComponent>(3, 5.0f, 5.0f);
  PhysicsSystem physics_system;
  physics_system.Run();
  auto* position_one = ecs::Get<PositionComponent>(1);
  auto* velocity_one = ecs::Get<VelocityComponent>(1);
  EXPECT_EQ(position_one->x_, 20);
  EXPECT_EQ(position_one->y_, 30);
  EXPECT_EQ(velocity_one->dx_, 2.0f);
  EXPECT_EQ(velocity_one->dy_, 6.0f);
  auto* position_two = ecs::Get<PositionComponent>(2);
  EXPECT_EQ(position_two->x_ == 10);
  EXPECT_EQ(position_two->y_ == 15);
  auto* position_three = ecs::Get<PositionComponent>(3);
  auto* velocity_three = ecs::Get<VelocityComponent>(3);
  EXPECT_EQ(position_three->x_, 10);
  EXPECT_EQ(position_three->y_, 10);
  EXPECT_EQ(velocity_three->dx_, 10.0f);
  EXPECT_EQ(velocity_three->dy_, 10.0f);
}

struct Foo {};
struct Bar {};
struct Baz {};

class MockFooBarBaz : public ecs::System<Foo, Bar, Baz> {
 public:
  MOCK_METHOD(void,
              OnEntity,
              (ecs::Entity, Foo&, Bar&, Baz&),
              (override));
};

class MockFooBar : public ecs::System<Foo, Bar> {
 public:
  MOCK_METHOD(void, OnEntity, (ecs::Entity, Foo&, Bar&), (override));
};

class MockFooBaz : public ecs::System<Foo, Baz> {
 public:
  MOCK_METHOD(void, OnEntity, (ecs::Entity, Foo&, Baz&), (override));
};


TEST(SystemTest, ExpectCorrectRunCounts) {
  using ::testing::Eq;
  using ::testing::_;
  // Entity 1 has all the components.
  ecs::Assign<Foo>(1);
  ecs::Assign<Bar>(1);
  ecs::Assign<Baz>(1);
  // Entity 2 has component Foo and Bar.
  ecs::Assign<Foo>(2);
  ecs::Assign<Bar>(2);
  // Entity 3 has components Foo and Bar.
  ecs::Assign<Foo>(3);
  ecs::Assign<Bar>(3);
  // Entity 4 has components Foo and Baz.
  ecs::Assign<Foo>(4);
  ecs::Assign<Baz>(4);
  MockFooBarBaz foo_bar_baz_mock;
  MockFooBar foo_bar_mock;
  MockFooBaz foo_baz_mock;
  // (Foo, Bar, Baz) called once with entity 1 only.
  EXPECT_CALL(foo_bar_baz_mock, OnEntity(Eq(1), _, _, _)).Times(1);
  foo_bar_baz_mock.Run();
  // (Foo, Bar) called three times with entity 1, 2 and 3 .
  EXPECT_CALL(foo_bar_mock, OnEntity(Eq(1), _, _));
  EXPECT_CALL(foo_bar_mock, OnEntity(Eq(2), _, _));
  EXPECT_CALL(foo_bar_mock, OnEntity(Eq(3), _, _));
  foo_bar_mock.Run();
  // (Foo, Baz) called twice with entity 1 and 4.
  EXPECT_CALL(foo_baz_mock, OnEntity(Eq(1), _, _));
  EXPECT_CALL(foo_baz_mock, OnEntity(Eq(4), _, _));
  foo_baz_mock.Run();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
