#pragma once

#include <cstdio>

#include "platform/macro.h"
#include "platform/x64_intrin.h"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
enum ShipAiGoals {
  kShipAiSpawnPod,
  kShipAiGoals = 64,
};
enum UnitAiGoals {
  kUnitAiPower = 0,
  kUnitAiMine,
  kUnitAiThrust,
  kUnitAiGoals = 64,
};
enum PodAiGoals {
  kPodAiLostPower,
  kPodAiGather,
  kPodAiUnload,
  kPodAiReturn,
  kPodAiApproach,
  kPodAiGoals = 64,
};
enum AsteroidAiGoals {
  kAsteroidAiImplode,
  kAsteroidAiDeplete,
};

constexpr float kDsqOperate = 50.f * 35.f;
constexpr float kDsqSelect = 25.f * 25.f;

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

  UseShip();

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
  for (int i = 0; i < kUsedShip; ++i) {
    // Ship already has a pod, no-op
    uint64_t think_flags = 0;
    if (!kUsedPod) {
      // Ship mining is powererd
      if (kShip[i].sys_mine >= 1.0f) {
        think_flags = FLAG(kShipAiSpawnPod);
      }
    }
    kShip[i].think_flags = think_flags;

    // Crew objectives
    uint64_t satisfied = 0;
    math::Vec2f module_position;
    tilemap::TileTypeWorldPosition(tilemap::kTilePower, &module_position);
    for (int j = 0; j < kUsedUnit; ++j) {
      if (dsq(kUnit[j].transform.position, module_position) < kDsqOperate)
        satisfied |= FLAG(kUnitAiPower);
    }

    if (kShip[i].sys_power >= 1.0f) {
      tilemap::TileTypeWorldPosition(tilemap::kTileEngine, &module_position);
      for (int j = 0; j < kUsedUnit; ++j) {
        if (dsq(kUnit[j].transform.position, module_position) < kDsqOperate)
          satisfied |= FLAG(kUnitAiThrust);
      }

      tilemap::TileTypeWorldPosition(tilemap::kTileMine, &module_position);
      for (int j = 0; j < kUsedUnit; ++j) {
        if (dsq(kUnit[j].transform.position, module_position) < kDsqOperate)
          satisfied |= FLAG(kUnitAiMine);
      }
    }

    kShip[i].crew_think_flags = satisfied;
  }

  for (int i = 0; i < kUsedUnit; ++i) {
    switch (kUnit[i].kind) {
      default:
      case 0:
        // Just takes orders
        kUnit[i].think_flags = 0;
        break;
      case 1:
        kUnit[i].think_flags = FLAG(kUnitAiPower);
        break;
      case 2:
        kUnit[i].think_flags = FLAG(kUnitAiMine);
        break;
      case 3:
        kUnit[i].think_flags = FLAG(kUnitAiThrust);
        break;
    };
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    if (asteroid->mineral_source < .5f) {
      asteroid->flags = FLAG(kAsteroidAiImplode);
    } else {
      asteroid->flags = 0;
    }
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    // Keep-state on AiReturn
    const uint64_t keep_state = pod->think_flags & FLAG(kPodAiReturn);
    math::Vec2f goal;
    uint64_t think_flags = 0;

    // Goal is to return home, unless overidden
    tilemap::TileTypeWorldPosition(tilemap::kTileMine, &goal);

    // TODO (AN): No ship/pod link exists yet
    if (kShip[0].sys_power < .5f) {
      think_flags |= FLAG(kPodAiLostPower);
    }

    // Stateful return home
    if (keep_state) {
      // Pod has finished unloading
      if (pod->mineral == 0) {
        // derp
        think_flags = ANDN(FLAG(kPodAiUnload), think_flags);
      } else {
        think_flags = keep_state;
        // In range of the ship
        if (dsq(goal, pod->transform.position) < 300.f) {
          // Unload the payload this tick!
          think_flags |= FLAG(kPodAiUnload);
        }
      }
    }
    // Begin the journey home
    else if (pod->mineral >= kPodMaxMineral) {
      math::Vec2f home;
      goal = home;
      think_flags |= FLAG(kPodAiReturn);
    } else {
      // Evaluate mining potential
      for (int i = 0; i < kUsedAsteroid; ++i) {
        Asteroid* asteroid = &kAsteroid[i];
        if (asteroid->mineral_source == 0) continue;

        // TODO: nearest dsq
        think_flags |= FLAG(kPodAiApproach);
        goal = asteroid->transform.position.xy();
        if (transform_dsq(&asteroid->transform, &pod->transform) < 900.f) {
          think_flags |= FLAG(kPodAiGather);
          asteroid->flags |= FLAG(kAsteroidAiDeplete);
        }
        break;
      }
    }

    pod->think_flags = think_flags;
    pod->goal = goal;
#if 1
    printf("pod think 0x%lx keep_state 0x%lx minerals %lu \n", think_flags,
           keep_state, pod->mineral);
#endif
  }
}

void
Decide()
{
  for (int i = 0; i < kUsedShip; ++i) {
    if (kShip[i].think_flags & FLAG(kShipAiSpawnPod)) {
      Pod* pod = UsePod();
      pod->transform.position = math::Vec3f(520.f, 600.f, 0.f);
    }

    if (kShip[i].crew_think_flags & FLAG(kUnitAiPower))
      kShip[i].sys_power += 0.01f;
    else
      kShip[i].sys_power += -0.01f;
    if (kShip[i].crew_think_flags & FLAG(kUnitAiMine))
      kShip[i].sys_mine += 0.01f;
    else
      kShip[i].sys_mine += -0.01f;
    if (kShip[i].crew_think_flags & FLAG(kUnitAiThrust))
      kShip[i].sys_engine += 0.01f;
    else
      kShip[i].sys_engine += -0.01f;

    kShip[i].sys_power = CLAMPF(kShip[i].sys_power, 0.0f, 1.0f);
    kShip[i].sys_mine = CLAMPF(kShip[i].sys_mine, 0.0f, 1.0f);
    kShip[i].sys_engine = CLAMPF(kShip[i].sys_engine, 0.0f, 1.0f);
  }

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
    uint64_t possible = ANDN(kShip[0].crew_think_flags, unit->think_flags);
    if (!possible) continue;

    uint64_t action = TZCNT(possible);
#if 1
    printf(
        "[ %d unit ] [ 0x%lx think ] [ 0x%lx possible ] ship crew_think_flags "
        "0x%lx \n",
        i, unit->think_flags, possible, kShip[0].crew_think_flags);
    printf("%lu action\n", action);
#endif
    math::Vec2f pos;
    switch (action) {
      case kUnitAiMine:
        if (TileTypeWorldPosition(tilemap::kTileMine, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
      case kUnitAiPower:
        if (TileTypeWorldPosition(tilemap::kTilePower, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
      case kUnitAiThrust:
        if (TileTypeWorldPosition(tilemap::kTileEngine, &pos)) {
          unit->command = Command{.type = Command::kMove, .destination = pos};
        }
        break;
    };
  }

  if (!kUsedAsteroid) {
    Asteroid* asteroid = UseAsteroid();
    asteroid->transform.position = math::Vec3f(800.f, 750.f, 0.f);
    asteroid->mineral_source = 200.f;
    asteroid->flags = 0;
  }

  // TODO (AN): Zero to avoid index slide and poor iteration performance
  for (int i = 0; i < kUsedAsteroid;) {
    Asteroid* asteroid = &kAsteroid[i];
    uint64_t action = TZCNT(asteroid->flags);
    switch (action) {
      case kAsteroidAiImplode:
        ReleaseAsteroid(i);
        break;
      default:
        ++i;
        break;
    }
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    uint64_t action = TZCNT(asteroid->flags);
    switch (action) {
      case kAsteroidAiDeplete:
        asteroid->mineral_source -= 1;
        break;
      default:
        break;
    }

    asteroid->transform.scale =
        math::Vec3f(1.f, 1.f, 1.f) * (asteroid->mineral_source / 200.f);
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];

    uint64_t action = TZCNT(pod->think_flags);
    uint64_t mineral = 0;
    math::Vec3f dir;
    switch (action) {
      case kPodAiLostPower:
        dir = pod->last_heading;
        break;
      case kPodAiApproach:
        dir = Normalize(pod->goal - pod->transform.position.xy());
        break;
      case kPodAiGather:
        mineral = MIN(1, kPodMaxMineral - pod->mineral);
        pod->mineral += mineral;
        break;
      case kPodAiReturn:
        dir = Normalize(pod->goal - pod->transform.position.xy());
        break;
      case kPodAiUnload:
        kShip[0].mineral += MIN(5, pod->mineral);
        pod->mineral -= MIN(5, pod->mineral);
        break;
    };

    pod->transform.position += dir * 2.0;
    pod->last_heading = dir.xy();
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
        transform->position += (dir * 1.f) + (TileAvoidWalls(start) * .15f);
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

uint64_t
SelectUnit(math::Vec3f world)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    if (dsq(unit->transform.position, world) < kDsqSelect) {
      return i;
    }
  }

  return kMaxUnit;
}

void
ToggleAi(uint64_t i)
{
  if (i >= kMaxUnit) return;

  printf("toggle ai %lu %d\n", i, kUnit[i].kind);
  if (kUnit[i].kind) {
    kUnit[i].kind = 0;
  } else {
    kUnit[i].kind = MAX(1, i);
  }
}

}  // namespace simulation
