#pragma once

#include <algorithm>
#include <unordered_map>

namespace search {
namespace internal {

// Score struct used in A* pathfinding cost maps.
// TODO: Change score to templated type. Where value_ is of type T
// instead of uint32_t for graphs that need to use real numbers. This
// is actually challengning because the type comes from the return
// value of the cost function given by PathTo. C++ Probably allows one
// to extract the type of a templated function somehow but I couldn't
// figure it out.
class Score {
public:
  Score() : value_(UINT32_MAX) {};
  Score(uint32_t value) : value_(value) {};
  uint32_t value_;
};

template <typename T>
class PathNode {
public:
  PathNode(const T& location, uint32_t cost, uint32_t heuristic) :
    location_(location), cost_(cost), heuristic_(heuristic) {};

  T location_;
  Score cost_;
  Score heuristic_;
};

template <typename T>
struct PathNodeComparator {
  bool operator() (const PathNode<T>& lhs, const PathNode<T>& rhs) {
    return lhs.heuristic_.value_ > rhs.heuristic_.value_;
  }
};

template <typename T>
void BuildPath(
    const T& target, std::unordered_map<T, T>& came_from,
    std::vector<T>& path) {
  T current = target;
  path.push_back(current);
  while (came_from.find(current) != came_from.end()) {
    current = came_from[current];
    path.push_back(current);
  }
  std::reverse(path.begin(), path.end());
}

}
}
