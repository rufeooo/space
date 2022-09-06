#pragma once

#include "phitu.cc"

namespace simulation
{

bool
ModuleBuilt(Module* module)
{
  if (!module) return false;
  return module->frames_building >= module->frames_to_build;
}

void
ModuleSetBuilt(Module* module)
{
  if (!module) return;
  module->frames_building = module->frames_to_build;
}

v3f
ModuleBounds(ModuleKind mkind)
{
  switch (mkind) {
    case kModPower:
      return v3f(15.f, 15.f, 15.f);
    case kModEngine:
      return v3f(15.f, 15.f, 15.f);
    case kModTurret:
      return v3f(15.f, 15.f, 15.f);
    case kModMine:
      return v3f(15.f, 15.f, 15.f);
    case kModBarrack:
      return v3f(45.f, 45.f, 15.f);
    case kModMedbay:
      return v3f(45.f, 45.f, 15.f);
    case kModWarp:
      return v3f(15.f, 15.f, 15.f);
    case kModDoor:
      return v3f(25.f, 25.f, 50.f);
    case kModCount:
    default:
      return v3f();
  }
  return v3f();
}

uint64_t
ModuleCost(ModuleKind mkind)
{
  switch (mkind) {
    case kModMine:
      return 400;
    case kModBarrack:
      return 400;
    case kModMedbay:
      return 400;
    case kModWarp:
      return 30;
    case kModTurret:
    case kModEngine:
    case kModPower:
    case kModDoor:
    case kModCount:
    default:
      return 0;
  }
  return 0;
}

const char*
ModuleName(ModuleKind mkind)
{
  switch (mkind) {
    case kModMine:
      return "Mining Drill";
    case kModBarrack:
      return "Barracks";
    case kModMedbay:
      return "Medbay";
    case kModTurret:
      return "Turret";
    case kModEngine:
      return "Engine";
    case kModPower:
      return "Power";
    case kModWarp:
      return "Warp";
    case kModDoor:
      return "Door";
    case kModCount:
    default:
      return "Unknown";
  }
  return "Unknown";
}

v3f
ModuleColor(ModuleKind mkind)
{
  switch (mkind) {
    case kModPower:
      return v3f(0.0f, 0.0f, 0.75f);
    case kModEngine:
      return v3f(1.0f, 0.0f, 1.f);
    case kModTurret:
      return v3f(0.3f, 0.3f, 0.0f);
    case kModMine:
      return v3f(0.0, 0.75f, 0.0f);
    case kModBarrack:
      return v3f(1.0, 0.0, 0.0);
    case kModMedbay:
      return v3f(1.0, 1.0, 1.0);
    case kModWarp:
      return v3f(0.6, 1.0, 1.0);
    case kModDoor:
      return v3f(0.4, 0.4, 0.7);
    case kModCount:
    default:
      return v3f();
  }
  return v3f();
}

// TODO: Probably add some grid checking here.
bool
ModuleCanBuild(ModuleKind mkind, Player* player)
{
  if (!player) return false;
  return player->mineral >= ModuleCost(mkind);
}

bool
ModuleNear(Module* module, v3f loc)
{
  // TODO: Take into consideration module bounds.
  if (LengthSquared(loc - module->position) < 50.f * 38.f) return true;
  return false;
}

bool
ModuleNearXY(Module* module, v3f loc)
{
  // TODO: Take into consideration module bounds.
  if (LengthSquared(loc.xy() - module->position.xy()) < 50.f * 38.f) return true;
  return false;
}

void
ModuleMineUpdate(Module* module)
{
  // Find the nearest asteroid
  float d = FLT_MAX;
  v3f p;
  Asteroid* a = nullptr;
  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    float nd = math::LengthSquared(asteroid->transform.position - module->position);
    if (nd < d) {
      d = nd;
      p = asteroid->transform.position;
      a = asteroid;
    }
  }

  if (!a) return;

  ProjectileCreate(p, module->position, 15.f, 2, kWeaponMiningLaser);
  kPlayer[module->player_index].mineral += .1f;
}

void
ModuleBarrackUpdate(Module* module)
{
  Player* player = &kPlayer[module->player_index];

  if (module->frames_training != kTrainIdle) {
    ++module->frames_training;
  }

  if (module->frames_training >= module->frames_to_train) {
    SpawnCrew(module->position, module->player_index, module->ship_index);
    module->frames_training = kTrainIdle;
  }

  // Barracks automatically trains a unit if it has enough minerals, it's
  // enabled and it's training state is not idle.
  if (module->frames_training != kTrainIdle || !module->enabled ||
      player->mineral < 50.f)
    return;

  module->frames_training = 0;
}

void
ModuleWarpUpdate(Module* module)
{
  Unit* unit = GetNearestUnit(module->position);
  if (!ModuleNear(module, unit->position)) return;

  // Find the nearest warp on a seperate tilemap from this one.
  Module* target_module = nullptr;
  float d = FLT_MAX;
  FOR_EACH_ENTITY(Module, nm, {
    if (nm == module) continue;
    if (!ModuleBuilt(nm)) continue;
    if (nm->ship_index == module->ship_index) continue;
    if (nm->mkind != kModWarp) continue;
    float nd = LengthSquared(nm->position - module->position);
    if (nd < d) {
      target_module = nm;
      d = nd;
    }
  });
  if (!target_module) return;

  LOGFMT("Warping crew to ship %i. control change %d->%d",
         target_module->ship_index, unit->control, target_module->control);
  TilemapModify mod(target_module->ship_index);
  BfsIterator iter = BfsStart(target_module->tile);
  while (BfsNextTile(&iter)) {
    int i = 0;
    for (; i < kUsedEntity; ++i) {
      Unit* unit = i2Unit(i);
      if (!unit) continue;
      if (unit->ship_index != target_module->ship_index) continue;
      if (unit->tile == *iter.tile) break;
    }
    if (i != kUsedEntity) continue;
    unit->position = TileToWorld(*iter.tile);
    unit->ship_index = target_module->ship_index;
    unit->player_index = target_module->player_index;
    unit->persistent_uaction = unit->uaction = kUaNone;
    unit->control = 0;
    break;
  }
}

// TODO(abrunasso): Lol - Do this a better way....
void
ModuleDoorUpdate(Module* module)
{
  constexpr float up = 80.f;
  constexpr float down = 15.f;
  Unit* unit = GetNearestUnit(module->position);
  if (!unit) return;
  if (!ModuleNearXY(module, unit->position)) {
    if (module->position.z > down) {
      module->position.z -= 1.f;
    }
    return;
  }
  if (module->position.z < up) {
    module->position.z += 1.f;
  }
}

void
ModuleUpdate(Module* module)
{
  TilemapModify modify(module->ship_index);
  switch (module->mkind) {
    case kModMine: {
      ModuleMineUpdate(module);
    } break;
    case kModBarrack: {
      ModuleBarrackUpdate(module);
    } break;
    case kModMedbay: {
    } break;
    case kModWarp: {
      ModuleWarpUpdate(module);
    } break;
    case kModDoor: {
      ModuleDoorUpdate(module);
    } break;
    default:
      break;
  }
}

}  // namespace simulation
