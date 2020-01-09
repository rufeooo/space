#pragma once

#ifndef PAGE
#define PAGE (4 * 1024)
#endif

#include <string.h>
#include "math/math.cc"
#include "platform/macro.h"

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
  uint64_t kind;
};

#define MAX_ENTITY ((PAGE) / sizeof(Entity))

static Entity game_entity[MAX_ENTITY] ALIGNAS(PAGE);
static Entity zero_entity;

inline bool
EntityExists(Entity* ent)
{
  return memcmp(ent, &zero_entity, sizeof(Entity)) != 0;
}

inline void
EntityReset(Entity* ent)
{
  memcpy(ent, &zero_entity, sizeof(Entity));
}

#define CONCAT(a, b) a##b
#define COMPONENT_RESET(e, c) (e->c = zero_##c)
#define COMPONENT_EXISTS(e, c) (memcmp(&e->c, &zero_##c, sizeof(zero_##c)) != 0)
