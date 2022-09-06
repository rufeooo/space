#pragma once

#include "math/vec.h"

#include "entity.cc"
#include "search.cc"

#include <cstdint>

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;

int
AssignPlayerId()
{
  static int id = 0;
  return (id++ % kPlayerCount);
}

float
v3fDsq(const v3f& dst, const v3f& src)
{
  v3f delta = dst - src;
  return LengthSquared(delta);
}

Unit*
GetUnit(v3f world)
{
  FOR_EACH_ENTITY(Unit, unit, {
    if (v3fDsq(unit->position, world) < kDsqSelect) {
      return unit;
    }
  });
  return nullptr;
}

Unit*
GetUnitTarget(uint64_t local_player, v3f world)
{
  FOR_EACH_ENTITY(Unit, unit, {
    if (FLAGGED(unit->control, local_player)) continue;
    if (v3fDsq(unit->position, world) < kDsqSelect) {
      return unit;
    }
  });

  return nullptr;
}

Unit*
SelectUnit(const Rectf& rect, int idx)
{
  Unit* unit = &kEntity[idx].unit;
  if (PointInRect(unit->position.xy(), rect)) {
    return unit;
  }
  return nullptr;
}

Module*
SelectModule(const Rectf& rect, int idx)
{
  if (kEntity[idx].type_id != kEeModule) return nullptr;

  Module* mod = &kEntity[idx].module;
  TilemapModify tm(mod->ship_index);
  // TODO: Take into consideration module bounds and do a rect/rect intersect.
  if (PointInRect(mod->position.xy(), rect)) {
    return mod;
  }
  return nullptr;
}

bool
InRange(Unit* source_unit, Unit* target_unit)
{
  float dsq = v3fDsq(source_unit->position, target_unit->position);
  float rsq = source_unit->attack_radius * source_unit->attack_radius;
  return dsq < rsq;
}

bool
InRange(uint64_t unit_id, uint64_t target_id)
{
  if (unit_id == kInvalidId || target_id == kInvalidId) return false;
  Unit* source_unit = FindUnit(unit_id);
  Unit* target_unit = FindUnit(target_id);
  if (!source_unit || !target_unit) return false;
  return InRange(source_unit, target_unit);
}

bool
ShouldAttack(Unit* unit, Unit* target)
{
  if (!unit || !target) return false;
  if (unit->alliance == target->alliance) return false;
  return true;
}

bool
ShouldAttack(uint64_t unit, uint64_t target)
{
  if (unit == kInvalidId || target == kInvalidId) return false;
  return ShouldAttack(FindUnit(unit), FindUnit(target));
}

Unit*
FindUnitInRangeToAttack(Unit* unit)
{
  FOR_EACH_ENTITY(Unit, target, {
    if (unit == target) continue;
    if (!ShouldAttack(unit, target)) continue;
    if (InRange(unit, target)) return target;
  });
  return nullptr;
}

Unit*
GetNearestEnemyUnit(Unit* unit)
{
  float d = FLT_MAX;
  Unit* target = nullptr;
  FOR_EACH_ENTITY(Unit, new_target, {
    float nd = v3fDsq(unit->position, new_target->position);
    if (nd < d && ShouldAttack(unit, new_target)) {
      target = new_target;
      d = nd;
    }
  });
  return target;
}

Unit*
GetNearestUnit(const v3f& pos)
{
  float d = FLT_MAX;
  Unit* target = nullptr;
  FOR_EACH_ENTITY(Unit, new_target, {
    float nd = v3fDsq(pos, new_target->position);
    if (nd < d) {
      target = new_target;
      d = nd;
    }
  });
  return target;
}

uint32_t
SpawnEnemy(v2i tile_position, uint64_t ship_index)
{
  // Uses raii to revert ship index back to whatever was set.
  TilemapModify tm(ship_index);
  Unit* enemy = UseEntityUnit();
  enemy->position = TilePosToWorld(tile_position);
  enemy->scale = v3f(0.25f, 0.25f, 0.f);
  enemy->ship_index = ship_index;
  enemy->player_index = kInvalidIndex;
  enemy->alliance = kEnemy;
  enemy->kind = kAlien;
  enemy->attack_radius = 30.f;
  enemy->speed = 0.5f;
  enemy->bounds = v3f(15.f, 15.f, 25.f);
  BB_SET(enemy->bb, kUnitBehavior, kUnitBehaviorAttackWhenDiscovered);
  return enemy->id;
}

void
SpawnAICrew(v3f world_position, uint64_t player_index, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  Unit* unit = UseEntityUnit();
  unit->position = world_position;
  unit->scale = v3f(0.25f, 0.25f, 0.f);
  unit->ship_index = ship_index;
  unit->player_index = player_index;
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->notify = 1;
  unit->bounds = v3f(17.f, 17.f, 25.f);
  BB_SET(unit->bb, kUnitBehavior, kUnitBehaviorCrewMember);
}

void
SpawnAICrew(v2i tile_position, uint64_t player_index, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  Unit* unit = UseEntityUnit();
  unit->position = TilePosToWorld(tile_position);
  unit->scale = v3f(0.25f, 0.25f, 0.f);
  unit->ship_index = ship_index;
  unit->player_index = player_index;
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->notify = 1;
  unit->bounds = v3f(17.f, 17.f, 25.f);
  BB_SET(unit->bb, kUnitBehavior, kUnitBehaviorCrewMember);
}

void
SpawnCrew(v3f world_position, uint64_t player_index, uint64_t ship_index)
{
  TilemapModify tm(ship_index);
  Unit* unit = UseEntityUnit();
  unit->position = world_position;
  unit->scale = v3f(0.25f, 0.25f, 0.f);
  unit->ship_index = ship_index;
  unit->player_index = player_index;
  unit->kind = kOperator;
  unit->spacesuit = 1;
  unit->notify = 1;
  unit->bounds = v3f(17.f, 17.f, 25.f);
}

void
SpawnCrew(v2i tile_position, uint64_t player_index, uint64_t ship_index)
{
  SpawnCrew(TilePosToWorld(tile_position), player_index, ship_index);
}

void
SpawnCrew(v3f world_position, uint64_t ship_index)
{
  SpawnCrew(world_position, AssignPlayerId(), ship_index);
}

void
SpawnCrew(v2i tile_position, uint64_t ship_index)
{
  SpawnCrew(TilePosToWorld(tile_position), AssignPlayerId(), ship_index);
}

bool
CanPathTo(Unit* unit, Unit* target)
{
  if (!unit || !target) return false;
  if (unit->ship_index != target->ship_index) return false;
  return PathTo(unit->tile, target->tile) != nullptr;
}

}  // namespace simulation
