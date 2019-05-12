#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "internal.h"

namespace search {

// PathTo calculates a shortest path from start to end inclusive using
// A*. The user is expected to provide a valid cost function and 
// heuristic function.
//
// Parameters -
//
//   start, end: Start and end node in the graph, must be hashable.
//
//   expand_func: A functor for expanding nodes from a specific node
//                on the graph. The function should return all
//                adjacent nodes that are reachable from the current
//                node. The signature should be:
//
//                std::vector<T> Expand(const T& node)
//
//   cost_func: The cost of going from node a to node b in the graph.
//              This number should be non-negative and should reresent
//              a total cost the user would like to minimize. The
//              signature should be:
//
//              uint32_t Cost(const T& a, const T& b)
//
//   heuristic_func: The estimated cost from the current node to the
//                   end node. The user is responsible for providing
//                   a well-behaved and admissible heuristic function.
//                   That is the the function should never overestimate
//                   the total cost from the current node to end. A
//                   good heuristic function is often Euclidean
//                   distance if the problem can be defined in such a
//                   manner that Euclidean distance makes sense. The
//                   signature should be.
//
//                   uint32_t Heuristic(const T& a, const T& end)
//
// Returns -
//  A list of type std::vector<T> that contains the shortest path
//  from start to finish including both start and finish. 
//
// Example Usage -
//
//  To generate a path on a line from start to end assuming from any
//  point on the line you can go up one or down one.
//
//  auto expand_func = [](int start) {
//    return std::vector<int>({start + 1, start - 1});
//  };
//  auto cost_func = [](int, int) { return 1; };
//  auto heuristic_func = [](int current, int end) { 
//    return end - current;
//  };
//  std::vector<int> path = search::PathTo(
//      start, end, expand_func, cost_func, heuristic_func);
//
//  Path contains 0-10.

template <typename T, typename E, typename C, typename H>
std::vector<T> PathTo(
    const T& start, const T& end,
    E expand_func, C cost_func, H heuristic_func) {
  std::vector<T> coords;
  std::priority_queue<PathNode<T>, std::vector<PathNode<T>>,
                      PathNodeComparator<T>> open;
  open.push(PathNode(start, 0, heuristic_func(start, end)));
  // Set of open list discoveries for quick lookup. Unordered map
  // because set uses tree and needs >,< operator.
  std::unordered_map<T, bool> openDiscovered;
  openDiscovered[start] = true;
  // All nodes that are already evaluated. Unordered map because set
  // uses tree and needs >,< operator.
  std::unordered_map<T, bool> closed;
  // Map used to move backwards from goal node to start to get
  // pstartath.
  std::unordered_map<T, T> came_from;
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
    std::vector<T> neighbors = expand_func(current.location_);
    // Loop over neighbors and evaluate state of each node in path.
    for (const auto& neighbor : neighbors) {
      // Ignore neighbors that have already been evaluated.
      if (closed.find(neighbor) != closed.end()) {
        continue;
      }
      auto cost = cost_func(current.location_, neighbor);
      PathNode pn(
        neighbor, cost, cost + heuristic_func(neighbor, end)
      );
      // If not in open list, add it for evaluation.
      if (openDiscovered.find(neighbor) == openDiscovered.end()) {
        open.push(pn);
        openDiscovered[neighbor] = true;
      } else if (pn.cost_.value_ > true_costs[neighbor].value_) {
        continue;
      }
      came_from[neighbor] = current.location_;
      true_costs[neighbor].value_ = pn.cost_.value_;
    }
  }
  return coords;
}

}
