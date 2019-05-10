#pragma once

namespace search {

// Score struct used in A* pathfinding cost maps.
class Score {
public:
  Score() : value_(UINT32_MAX) {};
  Score(uint32_t value) : value_(value) {};
  uint32_t value_;
};

template <typename T>
class PathNode {
public:
  PathNode(const T& location, uint32_t cost,
           uint32_t heuristic) :
    location_(location), cost_(cost), heuristic_(heuristic) {};

  T location_;
  Score cost_;
  Score heuristic_;
};

struct PathNodeComparator {
  bool operator() (const PathNode& lhs, const PathNode& rhs) {
    return lhs.heuristic_.value_ > rhs.heuristic_.value_;
  }
};

}
