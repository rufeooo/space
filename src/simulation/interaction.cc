
#include "entity.cc"
#include "ftl.cc"
#include "tilemap.cc"

namespace simulation
{
constexpr float kDsqSelect = 25.f * 25.f;

bool
ShipFtlReady()
{
  return kShip[0].sys_engine > .5f && kShip[0].mineral >= kFtlCost;
}

void
ShipFtlInit()
{
  FtlInit(&kShip[0].ftl);
}

uint64_t
SelectUnit(v3f world)
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
ControlUnit(uint64_t unit_id)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    bool is_unit = i == unit_id;
    kUnit[i].kind = !is_unit * (i + 1);
  }
}

void
PlaceModuleMine(v3f world_pos)
{
  v2i tile_pos = WorldToTilePos(world_pos.xy());
  if (!TileOk(tile_pos)) return;

  kTilemap.map[tile_pos.y][tile_pos.x].type = kTileMine;
}

}  // namespace simulation
