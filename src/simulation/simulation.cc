#pragma once

#include <cstdio>

#include "platform/macro.h"
#include "platform/x64_intrin.h"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
enum AiGoals {
  kAiPower = 0,
  kAiMine,
  kAiThrust,
  kAiGoals = 64,
};

bool
Initialize()
{
  math::Vec3f pos[] = {
      math::Vec3f(300.f, 300.f, 0.f), math::Vec3f(100.f, 130.f, 0),
      math::Vec3f(300.f, 400.f, 0), math::Vec3f(650.f, 460.f, 0)};
  const math::Vec3f scale = math::Vec3f(0.25f, 0.25f, 0.f);
  for (int i = 0; i < ARRAY_LENGTH(pos); ++i) {
    Unit* unit = UseUnit();
    unit->transform.position = pos[i];
    unit->transform.scale = scale;
    // Everybody is unique!
    unit->kind = i;
  }

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    printf("UNIT %i: %.2f,%.2f\n", i, unit->transform.position.x,
           unit->transform.position.y);
  }

  Asteroid* asteroid = UseAsteroid();
  asteroid->transform.position = math::Vec3f(400.f, 750.f, 0.f);

  tilemap::Initialize();

  return true;
}

bool
VerifyIntegrity()
{
  // TODO (AN): Checksum of kEntity compared to end of last frame
  return true;
}

void
Think()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    switch (kUnit[i].kind) {
      default:
      case 0:
        // Just takes orders
        kUnit[i].think_flags = 0;
        break;
      case 1:
        kUnit[i].think_flags = FLAG(kAiPower);
        break;
      case 2:
        kUnit[i].think_flags = FLAG(kAiMine);
        break;
      case 3:
        kUnit[i].think_flags = FLAG(kAiThrust);
        break;
    };
  }
}

void
Decide()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    // Busy unit
    if (unit->command.type != Command::kNone) {
      continue;
    }

    // Obedient Unit
    if (unit->think_flags == 0) {
      if (CountCommand()) {
        unit->command = PopCommand();
      }
      continue;
    }

    // Self-motivated Unit
    uint64_t possible = ANDN(kShip[0].satisfied_flags, unit->think_flags);
    if (!possible) continue;

    uint64_t action = TZCNT(possible);
#if 1
    printf(
        "[ %d unit ] [ 0x%lx think ] [ 0x%lx possible ] ship satisfied_flags "
        "0x%lx \n",
        i, unit->think_flags, possible, kShip[0].satisfied_flags);
    printf("%lu action\n", action);
#endif
    math::Vec2f pos;
    switch (action) {
      case kAiMine:
        if (WorldPositionOfTile(tilemap::kTileMine, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
      case kAiPower:
        if (WorldPositionOfTile(tilemap::kTilePower, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
      case kAiThrust:
        if (WorldPositionOfTile(tilemap::kTileEngine, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
    };
    kShip[0].satisfied_flags |= 1 << action;
  }
}

void
Update()
{
  Think();
  Decide();

  using namespace tilemap;

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    Transform* transform = &unit->transform;

    switch (unit->command.type) {
      case Command::kNone: {
      } break;
      case Command::kMine: {
      } break;
      case Command::kMove: {
        math::Vec2i start = WorldToTilePos(transform->position.xy());
        math::Vec2i end = WorldToTilePos(unit->command.destination);

        auto* path = search::PathTo(start, end);
        if (!path || path->size <= 1) {
          unit->command = {};
          continue;
        }

        math::Vec3f dest = TilePosToWorld(path->tile[1]);
        auto dir = math::Normalize(dest - transform->position.xy());
        transform->position += dir * 1.f;
      } break;
      default:
        break;
    }
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    asteroid->transform.position.x -= 1.0f;
    if (asteroid->transform.position.x < 0.f) {
      asteroid->transform.position.x = 800.f;
    }
  }
}

}  // namespace simulation
