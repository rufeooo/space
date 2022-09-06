#pragma once

#include <cstring>

#include "tilemap.cc"

namespace simulation
{
struct Path {
  v2i tile[kMapHeight * kMapWidth];
  int size;
};

struct Search {
  // Visited Nodes
  v2i path_map[kMapHeight][kMapWidth];
  // BFS queue.
  v2i queue[kMapHeight * kMapWidth];
  int queue_size;
  // The resulting path as calculated from the last call to PathTo.
  Path path;
};

struct BfsIterator {
  int neighbor_index = 0;
  int queue_index = 0;
  Tile* tile = nullptr;
};

static Search kSearch;

BfsIterator
BfsStart(v2i start)
{
  constexpr int N = kMapHeight * kMapWidth;
  memset(kSearch.path_map, 0, sizeof(v2i) * N);
  kSearch.queue_size = 0;
  kSearch.path.size = 0;
  BfsIterator itr;
  itr.neighbor_index = 0;
  itr.queue_index = 0;
  itr.tile = nullptr;
  if (!TileOk(start)) return itr;
  kSearch.queue[kSearch.queue_size++] = start;
  kSearch.path_map[start.y][start.x] = start;
  itr.tile = TilePtr(start);
  return itr;
}

// Performs a single step of the bfs expansion.
// Returns true when the next node is first discovered.
// Returns false when the next node does not exist, or has been seen.
INLINE bool
BfsStep(v2i from, BfsIterator* iter)
{
  const auto& path_map = kSearch.path_map;
  const int neighbor_index = iter->neighbor_index;
  const v2i neighbor = from + kNeighbor[neighbor_index % kMaxNeighbor];
  Tile* tile = TilePtr(neighbor);

  iter->queue_index = (neighbor_index + 1) / kMaxNeighbor;
  iter->neighbor_index += 1;
  iter->tile = tile;

  if (!tile) return false;
  if (path_map[neighbor.y][neighbor.x] != INVALID_TILE) return false;
  return true;
}

// Performs many steps of BFS, looking for a node that has not been seen.
//
// Returns true when more nodes may be searched.
// Returns false when all nodes have been searched.
INLINE bool
BfsNext(BfsIterator* iter)
{
  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  while (iter->queue_index < qsz) {
    v2i from = queue[iter->queue_index];
    if (BfsStep(from, iter)) {
      if (iter->tile->blocked) continue;

      path_map[iter->tile->cy][iter->tile->cx] = from;
      queue[qsz++] = v2i(iter->tile->cx, iter->tile->cy);
      break;
    }
  }

  return iter->queue_index < qsz;
}

Path*
PathTo(const v2i& start, const v2i& end)
{
  if (start == end) return nullptr;
  if (!TileOk(end)) return nullptr;
  if (!TileOk(start)) return nullptr;

  auto& path_map = kSearch.path_map;
  BfsIterator iter = BfsStart(start);
  while (BfsNext(&iter)) {
    if (path_map[end.y][end.x] != INVALID_TILE) {
      break;
    }
  }

  if (path_map[end.y][end.x] == INVALID_TILE) return nullptr;

  auto& path = kSearch.path;
  auto& psz = kSearch.path.size;
  path.tile[psz++] = end;
  while (path.tile[psz - 1] != start) {
    auto& prev = path.tile[psz - 1];
    path.tile[psz++] = kSearch.path_map[prev.y][prev.x];
  }
  // Reverse it
  for (int i = 0, last = psz - 1; i < last; ++i, --last) {
    auto t = path.tile[last];
    path.tile[last] = path.tile[i];
    path.tile[i] = t;
  }

  return &kSearch.path;
}

uint64_t
BfsRemoveOxygen(v2i start, const uint64_t limit)
{
  uint64_t count = 0;
  {
    Tile* start_tile = TilePtr(start);
    if (!start_tile) return 0;
    if (start_tile->nooxygen == 0) {
      start_tile->nooxygen = 1;
      ++count;
    }
  }

  BfsIterator iter = BfsStart(start);

  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  while (count < limit) {
    // No more tiles
    if (iter.queue_index == qsz) break;

    v2i from = queue[iter.queue_index];
    if (BfsStep(from, &iter)) {
      if (iter.tile->blocked) continue;

      if (iter.tile->nooxygen == 0) {
        iter.tile->nooxygen = 1;
        ++count;
      }
      path_map[iter.tile->cy][iter.tile->cx] = from;
      queue[qsz++] = v2i(iter.tile->cx, iter.tile->cy);
    }
  }

  return count;
}

void
BfsMutate(v3f origin, Tile keep_bits, Tile set_bits, float tile_dsq)
{
  v2i start;
  if (!WorldToTilePos(origin, &start)) return;
  {
    Tile* start_tile = TilePtr(start);
    if (!start_tile) return;
    *start_tile = TileAND(*start_tile, keep_bits);
    *start_tile = TileOR(*start_tile, set_bits);
  }

  auto& queue = kSearch.queue;
  auto& path_map = kSearch.path_map;
  int& qsz = kSearch.queue_size;

  BfsIterator iter = BfsStart(start);
  while (iter.queue_index != qsz) {
    v2i from = queue[iter.queue_index];
    if (BfsStep(from, &iter)) {
      if (iter.tile->blocked) continue;

      v3f neighbor_world = TileToWorld(*iter.tile);
      v3f world = neighbor_world - origin;
      float distance = LengthSquared(world);
      if (distance > tile_dsq) continue;

      *(iter.tile) = TileAND(*iter.tile, keep_bits);
      *(iter.tile) = TileOR(*iter.tile, set_bits);
      path_map[iter.tile->cy][iter.tile->cx] = from;
      queue[qsz++] = v2i(iter.tile->cx, iter.tile->cy);
    }
  }
}

}  // namespace simulation
