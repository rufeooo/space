#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "internal.h"

namespace search
{
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
std::vector<T>
PathTo(const T& start, const T& end, E expand_func, C cost_func,
       H heuristic_func)
{
  using internal::BuildPath;
  using internal::PathNode;
  using internal::PathNodeComparator;
  using internal::Score;
  std::vector<T> coords;
  std::priority_queue<PathNode<T>, std::vector<PathNode<T>>,
                      PathNodeComparator<T>>
      open;
  open.push(PathNode(start, 0, heuristic_func(start, end)));
  // Set of open list discoveries for quick lookup.
  std::unordered_set<T> open_discovered;
  open_discovered.insert(start);
  // All nodes that are already evaluated.
  std::unordered_set<T> closed;
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
    open_discovered.erase(current.location_);
    // Put into closed list.
    closed.insert(current.location_);
    // Get all of currents neighbors.
    std::vector<T> neighbors = expand_func(current.location_);
    // Loop over neighbors and evaluate state of each node in path.
    for (const auto& neighbor : neighbors) {
      // Ignore neighbors that have already been evaluated.
      if (closed.find(neighbor) != closed.end()) {
        continue;
      }
      auto cost = cost_func(current.location_, neighbor);
      PathNode pn(neighbor, cost, cost + heuristic_func(neighbor, end));
      // If not in open list, add it for evaluation.
      if (open_discovered.find(neighbor) == open_discovered.end()) {
        open.push(pn);
        open_discovered.insert(neighbor);
      } else if (pn.cost_.value_ > true_costs[neighbor].value_) {
        continue;
      }
      came_from[neighbor] = current.location_;
      true_costs[neighbor].value_ = pn.cost_.value_;
    }
  }
  return coords;
}

// BreadthFirst performs a breadth first search up until a maximum
// depth. A user provides an expasion function that both dictates
// when the expansion of bfs ends and/or which nodes to expand from the
// current.
//
// Parameters -
//
//    start: Which node in the graph to begin bfs from. Must be
//           hashable.
//
//    max_depth: The depth to perform a bfs to. The start node is
//               considered to be at depth 1. Therefore a max_depth
//               value of 0 will not expand. See search_test.cc for
//               documentation on how to use this value correctly.
//
//    expand_func: Function that dictates both when bfs should
//                 terminate and which nodes should be expanded. The
//                 expected signature is:
//
//                 std::vector<T> Expand(const T& from)
//
// Example Usage - See search_test.cc TEST_CASE Path exists using bfs.
template <typename T, typename E>
void
BreadthFirst(const T& start, int max_depth, E expand_func)
{
  std::queue<T> to_explore;
  // Used to avoid expanding nodes already explored.
  std::unordered_map<T, uint32_t> discovered;
  to_explore.push(start);
  // Start node is at depth 1.
  discovered[start] = 1;
  while (!to_explore.empty()) {
    const T& neighbor = to_explore.front();
    std::vector<T> neighbors = expand_func(neighbor);
    to_explore.pop();
    for (const auto& n : neighbors) {
      const auto& found = discovered.find(n);
      if (found != discovered.end()) continue;
      uint32_t current_depth = discovered[neighbor];
      if (current_depth <= max_depth) {
        to_explore.push(n);
        discovered[n] = current_depth + 1;
      }
    }
  }
}

}  // namespace search
