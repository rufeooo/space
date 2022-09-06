#pragma once

#include "math/vec.h"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
struct Scenario {
  enum Type {
    kGameScenario = 0,
    kCombatScenario,
    kSoloMission,
    kTwoShip,
    kEmptyScenario,
    kMaxScenario,
  };
  Type type;

  // Feature enabled in the scenario.
  unsigned ai : 1;
  unsigned ship : 1;
  unsigned asteroid : 1;
  unsigned missile : 1;
  unsigned pod : 1;
  unsigned tilemap : 2;
};

static Scenario kScenario;
constexpr const char* kScenarioNames[Scenario::kMaxScenario] = {
    "Game", "Combat", "Solo", "TwoShip", "Empty",
};

void
SpawnCrew(int count)
{
  v3f pos[] = {v3f(300.f, 300.f, 0.f), v3f(100.f, 130.f, 0),
               v3f(300.f, 400.f, 0), v3f(650.f, 460.f, 0),
               v3f(100.f, 577.f, 0.f)};
  const v3f scale = v3f(0.25f, 0.25f, 0.f);
  uint8_t attrib[CREWA_MAX] = {11, 10, 11, 10};
  for (int i = 0; i < ARRAY_LENGTH(pos) && i < count; ++i) {
    Unit* unit = UseIdUnit();
    unit->ship_index = 0;
    unit->transform.position = pos[i];
    unit->transform.scale = scale;
    memcpy(unit->acurrent, attrib, sizeof(attrib));
    unit->kind = kOperator;
    // Everybody is unique!
    unit->mskill = i;
    printf("Unit created %04.02fx %04.02fy\n", pos[i].x, pos[i].y);
  }
}

void
InitializeScenario(bool reset_features = true)
{
  int sid = kScenario.type;
  switch (sid) {
    case Scenario::kGameScenario: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
      }
      SpawnCrew(5);
      kUnit[4].spacesuit = 1;

    } break;
    case Scenario::kCombatScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      UseIdUnit();
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);

      UseIdUnit();
      kUnit[1].transform.position = v3f(400.f, 300.f, 0.f);
      kUnit[1].transform.scale = v3f(0.25f, 0.25f, 0.f);
      kUnit[1].kind = kMilitary;
      kUnit[1].alliance = kEnemy;
    } break;
    case Scenario::kSoloMission: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
        kScenario.ai = 1;
        kScenario.tilemap = 2;
      }
      // One unit
      UseIdUnit();
      kUnit[0].transform.position = v3f(300.f, 300.f, 0.f);
      kUnit[0].transform.scale = v3f(0.25f, 0.25f, 0.f);
    } break;
    case Scenario::kTwoShip: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
        kScenario.tilemap = 1;
      }
      SpawnCrew(5);
    } break;
    default:
    case Scenario::kEmptyScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
    } break;
  }
  kScenario.type = (Scenario::Type)sid;

  uint64_t grid_index = TilemapInitialize(kScenario.tilemap);
  // At least one ship
  kScenario.ship = 1;
  Ship* ship = UseShip();
  ship->grid_index = grid_index;
  // TODO (AN): works for now
  assert((ship-kShip) == grid_index);
  // Global resource pool until deeper into multiplayer
  UseResource();
  kResource[0].level = 1;

  switch (sid) {
    case Scenario::kSoloMission: {
      Tile keep_bits;
      memset(&keep_bits, 0xff, sizeof(Tile));
      keep_bits.explored = 0;
      keep_bits.exterior = 0;
      Tile set_bits;
      memset(&set_bits, 0x00, sizeof(Tile));
      float tile_world_distance = kMapWidth * kTileWidth;
      BfsMutate(kUnit[0].transform.position, keep_bits, set_bits,
                tile_world_distance * tile_world_distance);
    } break;
    case Scenario::kTwoShip: {
      // Create a second ship and tilemap
      Ship* s2 = UseShip();
      grid_index = TilemapInitialize(kScenario.tilemap);
      s2->grid_index = grid_index;
      kGrid[grid_index].transform.position = v2f(600.f, 600.f);
    } break;
  }
}

void
ResetScenario(bool reset_features)
{
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;
  kAutoIncrementIdUnit = kInvalidUnit + 1;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  InitializeScenario(reset_features);
}

bool
ScenarioOver()
{
  int sid = kScenario.type;
  switch (sid) {
    case Scenario::kGameScenario:
      if (kUsedShip == 0) {
        LOG("No ships remain.");
        return true;
      }
      if (kUsedUnit == 0) {
        LOG("The crew is dead.");
        return true;
      }
  }

  return false;
}

}  // namespace simulation
