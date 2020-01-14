#pragma once

#include <cstring>

#include "tilemap.cc"

namespace search
{
struct PathNode {
  math::Vec2i from;
  bool checked;
};

struct Path {
  math::Vec2i tile[tilemap::kMapHeight * tilemap::kMapWidth];
  int size;
};

struct Search {
  // Map used for pathfinding calculations.
  PathNode path_map[tilemap::kMapHeight][tilemap::kMapWidth];
  // BFS queue.
  math::Vec2i queue[tilemap::kMapHeight * tilemap::kMapWidth];
  int queue_size;
  int queue_ptr;
  // The resulting path as calculated from the last call to PathTo.
  Path path;
};

static Search kSearch;

struct NeighborSet {
  math::Vec2i neighbor[8];
  int count;  // Count of reachable neighbors.
};

// If the node should be checked.
void
AddIfOpen(const math::Vec2i& pos, NeighborSet* neighbor_set)
{
  if (!tilemap::TileOk(pos)) return;
  if (kSearch.path_map[pos.y][pos.x].checked == true) return;
  auto& tile = tilemap::kTilemap.map[pos.y][pos.x];
  if (tile.type != tilemap::kTileOpen) return;
  neighbor_set->neighbor[neighbor_set->count++] = pos;
}

void
Neighbors(const math::Vec2i& source, NeighborSet* neighbor_set)
{
  neighbor_set->count = 0;
  // Left/Right
  AddIfOpen(source + math::Vec2i(-1, 0), neighbor_set);
  AddIfOpen(source + math::Vec2i(1, 0), neighbor_set);
  // Up/Down
  AddIfOpen(source + math::Vec2i(0, 1), neighbor_set);
  AddIfOpen(source + math::Vec2i(0, -1), neighbor_set);
  // Diagnols
  AddIfOpen(source + math::Vec2i(1, 1), neighbor_set);
  AddIfOpen(source + math::Vec2i(-1, 1), neighbor_set);
  AddIfOpen(source + math::Vec2i(1, -1), neighbor_set);
  AddIfOpen(source + math::Vec2i(-1, -1), neighbor_set);
}

Path*
PathTo(const math::Vec2i& start, const math::Vec2i& end)
{
  if (!tilemap::TileOk(end)) return nullptr;
  if (!tilemap::TileOk(start)) return nullptr;

  constexpr int N = tilemap::kMapHeight * tilemap::kMapWidth;
  memset(kSearch.path_map, 0, sizeof(PathNode) * N);
  kSearch.queue_size = 0;
  kSearch.queue_ptr = 0;
  kSearch.path.size = 0;

  auto& queue = kSearch.queue;
  auto& qsz = kSearch.queue_size;
  auto& qptr = kSearch.queue_ptr;
  auto& path_map = kSearch.path_map;

  queue[qsz++] = start;
  path_map[start.y][start.x].from = start;
  path_map[start.y][start.x].checked = true;

  bool found = false;
  int i = 0;
  NeighborSet neighbor_set;
  while (qptr != qsz) {
    auto& node = queue[qptr++];
    ++i;
    if (node == end) {
      found = true;
      break;  // Found!
    }
    Neighbors(node, &neighbor_set);
    for (int i = 0; i < neighbor_set.count; ++i) {
      auto& neighbor = neighbor_set.neighbor[i];
      queue[qsz++] = neighbor;
      path_map[neighbor.y][neighbor.x].from = node;
      path_map[neighbor.y][neighbor.x].checked = true;
    }
  }
  if (!found) return nullptr;
  auto& path = kSearch.path;
  auto& psz = kSearch.path.size;
  path.tile[psz++] = end;
  while (path.tile[psz - 1] != start) {
    auto& prev = path.tile[psz - 1];
    path.tile[psz++] = path_map[prev.y][prev.x].from;
  }
  // Reverse it
  for (int i = 0, e = psz - 1; i < e; ++i, --e) {
    auto t = path.tile[e];
    path.tile[e] = path.tile[i];
    path.tile[i] = t;
  }
#if 0
  printf("Path is\n\n");
  for (int i = 0; i < psz; ++i) {
    printf("(%i,%i)\n", path.tile[i].x, path.tile[i].y);
  }
#endif
  return &kSearch.path;
}

}  // namespace search
