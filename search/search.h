#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "internal.h"

namespace search {

// PathTo -
// Parameters
//  Begin, End -> node of type T (T must be hashable).
//  A strategy for expanding nodes or type T.
//  A heuristic function of type F.
// Returns
//  A list of type T that represents the path. 

template <typename T, typename F, typename H>
std::vector<T> PathTo(
    const T& start, const T& end,
    F&& expand_func, H&& heuristic_func) {
  std::vector<T> coords;
  std::priority_queue<PathNode, std::vector<PathNode>,
                      PathNodeComparator> open;
  open.push(PathNode(start, 0, HeuristicEstimate(start, end)));
  // Set of open list discoveries for quick lookup. Unordered map
  // because set uses tree and needs >,< operator.
  std::unordered_map<T, bool> openDiscovered;
  openDiscovered[start] = true;
  // All nodes that are already evaluated. Unordered map because set
  // uses tree and needs >,< operator.
  std::unordered_map<T, bool> closed;

  // Map used to move backwards from goal node to start to get
  // pstartath.
  std::unordered_map<T, types::GridPoint> came_from;
  // The actual costs from the start node to a given node.
  std::unordered_map<T, Score> true_costs;
  // Cost from start to start is 0.
  true_costs[start].value_ = 0;
  while (!open.empty()) {
    // Back will return path node with least path cost.
    PathNode current = open.top();
    if (current.location_ == end) {
      BuildPath(current.location_, came_from, coords);
      return std::move(coords);
    }
    // Remove from open list.
    open.pop();
    openDiscovered.erase(current.location_);
    // Put into closed list.
    closed[current.location_] = true;
    // Get all of currents neighbors.
    std::vector<T> neighbors;
    world::Neighbors(current.location_, neighbors);
    // Loop over neighbors and evaluate state of each node in path.
    for (auto neighbor : neighbors) {
      // Ignore neighbors that have already been evaluated.
      if (closed.find(neighbor) != closed.end()) {
        continue;
      }
      // If the neighbors location equals end, this is our destination,
      // don't skip it or A* will never finish.
      // If the node shouldn't be expanded add it to the closed list
      // and continue.
      const world::Tile* tile = world::GetTile(neighbor);
      if (!tile) {
        continue;
      }

      if (neighbor != end && expand && !expand(*tile)) {
        closed[neighbor] = true;
        continue;
      }
      PathNode pn(
        neighbor,
        // Cost of current record to this node
        current.cost_.value_ + tile->path_cost,
        // Heuristic cost
        current.cost_.value_ + tile->path_cost +
        HeuristicEstimate(neighbor, end)
      );
      // If not in open list, add it for evaluation.
      if (openDiscovered.find(neighbor) == openDiscovered.end()) {
        open.push(pn);
        openDiscovered[neighbor] = true;
      }
      // If this is not a better path than one already found continue.
      else if (pn.cost_.value_ > true_costs[neighbor].value_) {
        continue;
      }
      came_from[neighbor] = current.location_;
      true_costs[neighbor].value_ = pn.cost_.value_;
    }
  }
  return coords;
}

void bfs(const types::GridPoint& start,
    std::function<bool(const world::Tile& tile)> expand);

bool NearestPathablePoint(const types::GridPoint& source,
    const std::vector<types::GridPoint>& tiles,
    types::GridPoint& out);

    // NOTE:
    // For bfs search, 
    //   if the comparator returns true the bfs will stop expanding at the first found instance.
    //   if the comparator returns false it will continue to expand all nodes.
    // Therefore, return true if you want to find a single instance of a search, false otherwise.

    // Returns true if any tile within the depth of a breadth first search meets the
    // criteria given by the comparator.
    //bool bfs(const sf::Vector3i& start,
    //    uint32_t depth,
    //    world_map::TileMap& tile_map,
    //    std::function<bool(const Tile& tile)> comparator);

    //// Run bfs for each unit to depth.
    //bool bfs_units(const sf::Vector3i& start,
    //    uint32_t depth,
    //    world_map::TileMap& tile_map,
    //    std::function<bool(const Unit&)> comparator);

    //// Run bfs for each city to depth.
    //bool bfs_cities(const sf::Vector3i& start,
    //    uint32_t depth,
    //    world_map::TileMap& tile_map,
    //    std::function<bool(const City&)> comparator);

    //// Run bfs for each improvement to depth.
    //bool bfs_improvements(const sf::Vector3i& start,
    //    uint32_t depth,
    //    world_map::TileMap& tile_map,
    //    std::function<bool(const Improvement&)> comparator);

    //// Run bfs for each resource to depth.
    //bool bfs_resources(const sf::Vector3i& start,
    //    uint32_t depth,
    //    world_map::TileMap& tile_map,
    //    std::function<bool(const Resource&)> comparator);

}
