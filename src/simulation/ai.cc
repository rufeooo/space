#pragma once

#include "entity.cc"
#include "module.cc"
#include "tilemap.cc"

namespace simulation
{
// AI Will wander to adjacent tiles.
void
AIWander(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  Tile t = TileNeighbor(unit->tile, rand());
  BB_SET(unit->bb, kUnitDestination, t);
  unit->uaction = kUaMove;
}

void
AIDefend(Unit* unit)
{
  if (unit->uaction != kUaNone) return;

  Unit* target = GetNearestEnemyUnit(unit);
  if (!target) return;

  if (!InRange(unit, target)) return;

  BB_SET(unit->bb, kUnitTarget, target->id);
  unit->uaction = kUaAttack;
}

// AI Will wander to adjacent tiles until it finds someone it can attack or
// a unit attacks it - in which case it will attack back.
void
AISimpleBehavior(Unit* unit)
{
  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  Unit* nearby_target = FindUnitInRangeToAttack(unit);
  if (nearby_target) {
    BB_SET(unit->bb, kUnitTarget, nearby_target->id);
    unit->uaction = kUaAttack;
    return;
  }

  AIWander(unit);
}

void
AIAttackInterrupt(Unit* unit)
{
  if (unit->uaction != kUaAttack) return;

  // If the AI discovers a nearer target than the one it is attacking it
  // will change targets.
  const uint32_t* current_target;
  if (!BB_GET(unit->bb, kUnitTarget, current_target)) return;

  Unit* nearest_target = GetNearestEnemyUnit(unit);
  if (!nearest_target) return;

  // Nearest target remains the one it is attacking.
  if (nearest_target->id == *current_target) return;
  // Otherwise switch to new nearest.
  BB_SET(unit->bb, kUnitTarget, nearest_target->id);
}

void
AIAttackWhenDiscovered(Unit* unit)
{
  AIAttackInterrupt(unit);

  // Non interrupting behavior.
  if (unit->uaction != kUaNone) return;

  if (unit->tile.shroud && !unit->tile.visible) return;

  Unit* target = GetNearestEnemyUnit(unit);
  if (!target) return;

  BB_SET(unit->bb, kUnitTarget, target->id);
  unit->uaction = kUaAttack;
}

// Crew members build unbuilt modules.
// Otherwise - find a random module, go to it, stay there for a bit. Repeat.
void
AICrewMember(Unit* unit)
{
  if (unit->uaction != kUaNone) return;

  const int* timer;
  // Timer should perhaps be a global AI construct that gets updated for all
  // units that contain active timers?
  if (BB_GET(unit->bb, kUnitTimer, timer) && *timer > 0) {
    // If timer is set the crew member is working on something.
    int new_time = (*timer) - 1;
    if (new_time <= 0) {
      BB_REM(unit->bb, kUnitTimer);
    } else {
      BB_SET(unit->bb, kUnitTimer, new_time);
    }
    return;
  }

  // Prioritize building unbuilt modules.
  Module* target_mod = nullptr;
  FOR_EACH_ENTITY(Module, mod, {
    if (mod->ship_index != unit->ship_index) continue;
    if (!ModuleBuilt(mod)) {
      target_mod = mod;
      break;
    }
  });

  // Find a random module.
  if (!target_mod) {
    int rand_val = rand() % kUsedEntity;
    for (int i = 0; i < kUsedEntity; ++i) {
      uint64_t idx = (rand_val + i) % kUsedEntity;
      Module* mod = i2Module(idx);
      if (!mod) continue;
      if (mod->ship_index != unit->ship_index) continue;
      if (mod->mkind == kModEngine || mod->mkind == kModMine ||
          mod->mkind == kModDoor) {
        target_mod = mod;
        break;
      }
    }
  }

  if (!target_mod) return;

  // Go to the mod and stay there for timer.
  BB_SET(unit->bb, kUnitDestination, target_mod->tile);
  unit->uaction = kUaMove;
  int init_timer = 200;
  BB_SET(unit->bb, kUnitTimer, init_timer);
}

void
AIThink(Unit* unit)
{
  if (!unit) return;

  // Units should defend themselves when enemies are nearby.
  AIDefend(unit);

  const int* behavior;
  if (!BB_GET(unit->bb, kUnitBehavior, behavior)) return;
  switch (*behavior) {
    case kUnitBehaviorSimple: {
      AISimpleBehavior(unit);
    } break;
    case kUnitBehaviorAttackWhenDiscovered: {
      AIAttackWhenDiscovered(unit);
    } break;
    case kUnitBehaviorCrewMember: {
      AICrewMember(unit);
    } break;
    default:
      break;
  }

  // Clear AI knowledge that should be learned per tick.
  BB_REM(unit->bb, kUnitAttacker);
}

}  // namespace simulation
