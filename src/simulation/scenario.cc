#pragma once

#include "math/vec.h"
#include "network/network.cc"

#include "entity.cc"
#include "search.cc"

namespace simulation
{
// Do not add new fields to this struct you are not alright with being memset
// to 0xff or 0x0.
struct Scenario {
  enum Type {
    kTwoShip = 0,
    kCombatScenario,
    kSoloMission,
    kEmptyScenario,
    kMaxScenario,
  };
  Type type;

  // Feature enabled in the scenario.
  unsigned ship : 1;
  unsigned asteroid : 1;
  unsigned missile : 1;
  unsigned pod : 1;
  unsigned invasion : 1;
};

static Scenario kScenario;
static uint64_t kFrame;

constexpr const char* kScenarioNames[Scenario::kMaxScenario] = {
    "TwoShip", "Combat", "Solo", "Empty",
};

void
ScenarioSpawnRandomModule(ModuleKind kind, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    if (module->mkind != kind) continue;
    if (module->ship_index != ship_index) continue;
    if (ModuleBuilt(module)) continue;
    // TODO: These correspond now - will they always?
    ModuleSetBuilt(module);
    return;
  }
}

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
    unit->spacesuit = 1;
    // Everybody is unique!
    unit->mskill = i;
    unit->player_id = AssignPlayerId();
  }
}

void
TilemapUnexplored(v3f world_position)
{
  Tile keep_bits;
  memset(&keep_bits, 0xff, sizeof(Tile));
  keep_bits.explored = 1;
  keep_bits.exterior = 0;
  Tile set_bits;
  memset(&set_bits, 0x00, sizeof(Tile));
  float tile_world_distance = kMapWidth * kTileWidth;
  BfsMutate(world_position, keep_bits, set_bits,
            tile_world_distance * tile_world_distance);
}

void
ScenarioInitialize(bool reset_features = true)
{
  int sid = kScenario.type;
  TilemapType tilemap_type = kTilemapEmpty;
  TilemapSet(0);

  // Build ship.
  uint64_t grid_index;
  switch (sid) {
    case Scenario::kCombatScenario: {
      grid_index = TilemapInitialize(kTilemapEmpty);
      // At least one ship
      kScenario.ship = 1;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
    case Scenario::kTwoShip: {
      grid_index = TilemapInitialize(kTilemapShip);
      // At least one ship
      kScenario.ship = 2;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
      Ship* s2 = UseShip();
      grid_index = TilemapInitialize(kTilemapShip);
      s2->grid_index = grid_index;
      s2->level = 1;
      kGrid[grid_index].transform.position = v2f(0.f, 800.f);
    } break;
    case Scenario::kSoloMission: {
      grid_index = TilemapInitialize(kTilemapShip);
      // At least one ship
      kScenario.ship = 1;
      Ship* ship = UseShip();
      ship->grid_index = grid_index;
      ship->pod_capacity = 1;
      ship->level = 1;
    } break;
  }

  // Spawn units / modules.
  switch (sid) {
    case Scenario::kCombatScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      ScenarioSpawnCrew(v2i(5, 12), 0);
      ScenarioSpawnEnemy(v2i(10, 12), 0);
    } break;
    case Scenario::kSoloMission: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }
      ScenarioSpawnCrew(v2i(8, 12), 0);
      ScenarioSpawnEnemy(v2i(10, 15), 0);
      ScenarioSpawnEnemy(v2i(10, 20), 0);
      ScenarioSpawnEnemy(v2i(4, 20), 0);
    } break;
    case Scenario::kTwoShip: {
      if (reset_features) {
        memset(&kScenario, 0xff, sizeof(kScenario));
        kScenario.missile = 0;
        kScenario.invasion = 0;
      }
      TilemapUnexplored(TilemapWorldCenter());
      ScenarioSpawnCrew(v2i(5, 23), 0);
      ScenarioSpawnCrew(v2i(5, 7), 1);
      ScenarioSpawnRandomModule(kModPower, 1);
      ScenarioSpawnRandomModule(kModPower, 0);
      ScenarioSpawnRandomModule(kModEngine, 1);
      ScenarioSpawnRandomModule(kModEngine, 1);
      ScenarioSpawnRandomModule(kModEngine, 0);
      ScenarioSpawnRandomModule(kModEngine, 0);
    } break;
    default:
    case Scenario::kEmptyScenario: {
      if (reset_features) {
        memset(&kScenario, 0, sizeof(kScenario));
      }

    } break;
  }
  // Always set unexplored/interior of ship
  TilemapUnexplored(TilemapWorldCenter());

  auto dims = window::GetWindowSize();
  for (int i = 0; i < kPlayerCount; ++i) {
    auto* player = UsePlayer();
    player->ship_index = i;
    player->camera.viewport.x = kNetworkState.player_info[i].window_width;
    player->camera.viewport.y = kNetworkState.player_info[i].window_height;
    player->resource.mineral = 400;
    player->resource.frame = 400;
  }
  LOGFMT("Player count: %lu", kUsedPlayer);

  for (int i = 0; i < kUsedPlayer; ++i) {
    TilemapSet(kPlayer[i].ship_index);
    camera::InitialCamera(&kPlayer[i].camera);
    camera::Move(&kPlayer[i].camera, TilemapWorldCenter());
  }
}  // namespace simulation

void
ScenarioReset(bool reset_features)
{
  kFrame = 0;
  // TODO (AN): GAME_QUEUE not in the registry
  kReadCommand = 0;
  kWriteCommand = 0;
  kAutoIncrementIdUnit = kInvalidUnit + 1;

  for (int i = 0; i < kUsedRegistry; ++i) {
    *kRegistry[i].memb_count = 0;
  }
  ScenarioInitialize(reset_features);
}

bool
ScenarioOver()
{
  return false;
}



}  // namespace simulation
