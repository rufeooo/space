#pragma once

#ifndef PAGE
#define PAGE (4 * 1024)
#endif

#include <string.h>
#include "math/math.cc"

struct DestinationComponent {
  math::Vec2f position;
};
static DestinationComponent zero_destination;

struct TransformComponent {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
  math::Vec3f prev_position;
};
static TransformComponent zero_transform;

struct Entity {
  DestinationComponent destination;
  TransformComponent transform;
};

#define MAX_ENTITY ((PAGE) / sizeof(Entity))

static Entity game_entity[MAX_ENTITY] __attribute__((aligned(PAGE)));
static Entity zero_entity;

inline bool
EntityExists(int entity)
{
  return memcmp(&game_entity[entity], &zero_entity, sizeof(Entity)) != 0;
}

inline void
EntityReset(int entity)
{
  memcpy(&game_entity[entity], &zero_entity, sizeof(Entity));
}

#define CONCAT(a, b) a##b
#define COMPONENT_RESET(i, c) (game_entity[i].c = zero_##c)
#define COMPONENT_EXISTS(i, c) \
  (memcmp(&game_entity[i].c, &zero_##c, sizeof(zero_##c)) != 0)
