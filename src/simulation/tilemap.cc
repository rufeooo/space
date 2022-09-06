#pragma once

#include <cassert>
#include <climits>
#include <cstdio>

#include "math/vec.h"

#include "entity.cc"

namespace simulation
{
constexpr float kTileWidth = 25.0f;
constexpr float kInverseTileWidth = 1.0f / kTileWidth;
constexpr float kTileHeight = 25.0f;
constexpr float kInverseTileHeight = 1.0f / kTileHeight;

Tile* kTilemap;
v2f kTilemapWorldOffset;

enum TileType {
  kTileOpen = 0,
  kTileBlock = 1,
  kTileModule = 2,
  kTilePower = kTileModule + kModPower,
  kTileEngine = kTileModule + kModEngine,
  kTileMine = kTileModule + kModMine,
  kTileTurret = kTileModule + kModTurret,
  kTileVacuum = 6,
  kTileConsumable = 7,
};

static_assert(sizeof(Tile) == sizeof(uint32_t),
              "Sync TileAND with the new Tile size");
INLINE Tile
TileAND(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t*)&lhs & *(uint32_t*)&rhs;
  return *(Tile*)&res;
}

INLINE Tile
TileOR(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t*)&lhs | *(uint32_t*)&rhs;
  return *(Tile*)&res;
}

#define INVALID_TILE v2i{0, 0};

constexpr int kMaxNeighbor = 8;
static const v2i kNeighbor[kMaxNeighbor] = {
    v2i(-1, 0), v2i(1, 0),  v2i(0, 1),  v2i(0, -1),
    v2i(1, 1),  v2i(-1, 1), v2i(1, -1), v2i(-1, -1),
};

// Returns the minimum position of the tile.
v2f
TilePosToWorld(const v2i& pos)
{
  return {
      kTilemapWorldOffset.x + ((float)pos.x * kTileWidth) + kTileWidth * .5f,
      kTilemapWorldOffset.y + ((float)pos.y * kTileHeight) + kTileHeight * .5f};
}

// Returns the minimum position of the tile.
v2f
TileToWorld(const Tile& tile)
{
  return TilePosToWorld(v2i(tile.cx, tile.cy));
}

v2i
WorldToTilePos(const v3f& pos)
{
  v2f relpos = pos.xy() - kTilemapWorldOffset;
  int x = (int)(relpos.x * kInverseTileWidth);
  int y = (int)(relpos.y * kInverseTileHeight);
  return {x, y};
}

// Returns true for positions that exist as a tile in kTilemap
bool
TileOk(v2i pos)
{
  if (pos.x < 0) return false;
  if (pos.x >= kMapWidth) return false;
  if (pos.y < 0) return false;
  if (pos.y >= kMapHeight) return false;
  return true;
}

Tile*
TilePtr(const v2i& pos)
{
  if (!TileOk(pos)) return nullptr;
  return &kTilemap[pos.y * kMapWidth + pos.x];
}

// Returns kTileBlock for non-existent tiles, and TileType otherwise.
bool
TileBlockedSafe(const v2i& pos)
{
  Tile* tile = TilePtr(pos);
  return !tile || tile->blocked;
}

// Returns any neighbor of type kTileOpen
v2i
TileOpenAdjacent(const v2i pos)
{
  Tile* tile = TilePtr(pos);
  assert(tile);
  if (!tile->blocked) return pos;

  for (int i = 0; i < kMaxNeighbor; ++i) {
    v2i neighbor = pos + kNeighbor[i];
    tile = TilePtr(neighbor);
    if (!tile) continue;

    if (!tile->blocked) return neighbor;
  }

  return INVALID_TILE;
}

v3f
TileAvoidWalls(const v2i pos)
{
  v2i avoidance = {};
  for (int i = 0; i < kMaxNeighbor; ++i) {
    v2i neighbor = pos + kNeighbor[i];
    Tile* tile = TilePtr(neighbor);
    if (!tile) continue;
    if (tile->blocked) {
      v2i away = (pos - neighbor);
      avoidance += away;
    }
  }

  return v3f(avoidance.x, avoidance.y, 0.0f);
}

v2f
TileVacuum(const v2i pos)
{
  v2f posf(pos.x * 1.f, pos.y * 1.f);
  v2f center(kMapWidth * 0.5f, kMapHeight * 0.5f);
  return Normalize(posf - center);
}

math::Rectf
TilemapWorldBounds()
{
  v2i min = {0, 0};
  v2i max = {kMapWidth, kMapHeight};
  math::Rectf ret;
  v2f center(kTileWidth * .5f, kTileHeight * .5f);
  ret.min = TilePosToWorld(min) - center;
  ret.max = TilePosToWorld(max) - center;

  return ret;
}

v3f
TilemapWorldCenter()
{
  v2i min = {0, 0};
  v3f world_min = TilePosToWorld(min);
  world_min +=
      v2f(kMapWidth * kTileWidth * .5f, kMapHeight * kTileHeight * .5f);
  return world_min;
}

void
TilemapSet(uint64_t grid_idx)
{
  if (grid_idx > kUsedGrid) {
    kTilemap = nullptr;
    return;
  }

  kTilemap = (Tile*)kGrid[grid_idx].tilemap;
  kTilemapWorldOffset = kGrid[grid_idx].transform.position.xy();
}

uint64_t
TilemapInitialize(int tilemap_style)
{
  memset(&kTilemap, 0, sizeof(kTilemap));

  Grid* grid = UseGrid();
  uint64_t grid_index = grid - kGrid;
  grid->transform.position = v3f();
  TilemapSet(grid_index);

  if (!tilemap_style) {
    return grid_index;
  }

  // clang-format off
static int kDefaultMap[kMapHeight][kMapWidth] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 7, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 7, 0, 0, 0, 0, 0, 5, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 3, 0, 0, 7, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

  // clang-format on
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = TilePtr(v2i(j, i));
      tile->cx = j;
      tile->cy = i;
      tile->blocked = (kDefaultMap[i][j] == kTileBlock);
      tile->nooxygen = 0;
      tile->explored = 1;
      tile->exterior = 1;

      // No modules for tilemap_style 2
      // Shroud enabled
      // Fog enabled
      if (tilemap_style == 2) {
        if (kDefaultMap[i][j] == kTileConsumable) {
          Consumable* c = UseConsumable();
          c->ship_index = grid_index;
          c->cx = tile->cx;
          c->cy = tile->cy;
          c->minerals = i * j % 89;
          if (kDefaultMap[i][j - 1] == kTileBlock) {
            c->cryo_chamber = 1;
          }
        } else if (kDefaultMap[i][j] == kTilePower) {
          Module* t = UseModule();
          t->ship_index = grid_index;
          t->cx = tile->cx;
          t->cy = tile->cy;
          t->mkind = kModPower;
        }

        continue;
      }
      switch (kDefaultMap[i][j]) {
        case kTilePower:
        case kTileEngine:
        case kTileMine:
        case kTileTurret: {
          Module* t = UseModule();
          t->ship_index = grid_index;
          t->cx = tile->cx;
          t->cy = tile->cy;
          t->mkind = kDefaultMap[i][j] - kTileModule;
        } break;
      };
    }
  }

  return grid_index;
}

void
TilemapUpdate(int tilemap_style)
{
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      Tile* tile = TilePtr(v2i(j, i));
      tile->shroud = !tile->exterior;
    }
  }
}

}  // namespace simulation
