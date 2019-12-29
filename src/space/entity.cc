
#pragma once

#ifndef PAGE
#define PAGE (4 * 1024)
#endif

#include "math/math.cc"

struct DestinationComponent {
  math::Vec2f position;
};
static DestinationComponent zero_destination;
struct RectangleComponent {
  math::Vec4f color;
};
static RectangleComponent zero_rectangle;
struct TriangleComponent {
  math::Vec4f color;
};
static TriangleComponent zero_triangle;
struct LineComponent {
  math::Vec3f start;
  math::Vec3f end;
  math::Vec4f color;
};
static LineComponent zero_line;
struct GridComponent {
  float width;   // Width of the grid component.
  float height;  // Height of the grid component.
  math::Vec4f color;
};
static GridComponent zero_grid;
struct TransformComponent {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
  math::Vec3f prev_position;
};
static TransformComponent zero_transform;

struct Entity {
  DestinationComponent destination;
  RectangleComponent rectangle;
  TriangleComponent triangle;
  LineComponent line;
  GridComponent grid;
  TransformComponent transform;
};
#define MAX_ENTITY ((PAGE) / sizeof(Entity))

static Entity game_entity[MAX_ENTITY] __attribute__((aligned(PAGE)));
static Entity zero_entity;

#define NO_ENTITY(i) \
  (memcmp(&game_entity[i], &zero_entity, sizeof(Entity)) == 0)
#define RESET_ENTITY(i) (memcpy(&game_entity[i], &zero_entity, sizeof(Entity)))
#define RESET_COMPONENT(i, c) (game_entity[i].c = zero_##c)

#define CONCAT(a, b) a##b
#define NO_COMPONENT(i, c) \
  (memcmp(&game_entity[i].c, &zero_##c, sizeof(zero_##c)) == 0)
