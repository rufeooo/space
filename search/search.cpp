#include "search.h"
#include "world.h"
#include "tile.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>

namespace {

// Score struct used in A* pathfinding cost maps.
class Score {
public:
    Score() : value_(UINT32_MAX) {};
    Score(uint32_t value) : value_(value) {};
    uint32_t value_;
};

class PathNode {
public:
    PathNode(const types::GridPoint& location, uint32_t cost, uint32_t heuristic) :
        location_(location)
        , cost_(cost)
        , heuristic_(heuristic) {};

    types::GridPoint location_;
    Score cost_;
    Score heuristic_;
};

struct PathNodeComparator {
    bool operator() (const PathNode& lhs, const PathNode& rhs) {
        return lhs.heuristic_.value_ > rhs.heuristic_.value_;
    }
};

void Set(Score& score, uint32_t value) {
    score.value_ = value;
}

uint32_t HeuristicEstimate(const types::GridPoint& from, const types::GridPoint& goal) {
    return DistanceSquared(from, goal);
}

void BuildPath(const types::GridPoint& target,
    std::unordered_map<types::GridPoint, types::GridPoint, VectorHash>& came_from,
    std::vector<types::GridPoint>& path) {
    types::GridPoint current = target;
    path.push_back(current);
    while (came_from.find(current) != came_from.end()) {
        current = came_from[current];
        path.push_back(current);
    }
    // Path in reverse order so reverse it.
    std::reverse(path.begin(), path.end());
}
}

// Calculates the path with the cheapest cumulative tile path cost
std::vector<types::GridPoint> search::PathTo(const types::GridPoint& start,
    const types::GridPoint& end,
    std::function<bool(const world::Tile& tile)> expand) {
    std::vector<types::GridPoint> coords;
    std::priority_queue<PathNode, std::vector<PathNode>, PathNodeComparator> open;
    open.push(PathNode(start, 0, HeuristicEstimate(start, end)));
    // Set of open list discoveries for quick lookup. Unordered map because set uses tree and needs >,< operator.
    std::unordered_map<types::GridPoint, bool, VectorHash> openDiscovered;
    openDiscovered[start] = true;
    // All nodes that are already evaluated. Unordered map because set uses tree and needs >,< operator.
    std::unordered_map<types::GridPoint, bool, VectorHash> closed;

    // Map used to move backwards from goal node to start to get pstartath.
    std::unordered_map<types::GridPoint, types::GridPoint, VectorHash> came_from;
    // The actual costs from the start node to a given node.
    std::unordered_map<types::GridPoint, Score, VectorHash> true_costs;
    // Cost from start to start is 0.
    Set(true_costs[start], 0);

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
        std::vector<types::GridPoint> neighbors;
        world::Neighbors(current.location_, neighbors);
        // Loop over neighbors and evaluate state of each node in path.
        for (auto neighbor : neighbors) {
            // Ignore neighbors that have already been evaluated.
            if (closed.find(neighbor) != closed.end()) continue;
            // If the neighbors location equals end, this is our destination, don't skip it or A* will never finish.
            // If the node shouldn't be expanded add it to the closed list and continue.
            const world::Tile* tile = world::GetTile(neighbor);
            if (!tile) continue;

            if (neighbor != end && expand && !expand(*tile)) {
                closed[neighbor] = true;
                continue;
            }

            PathNode pn(neighbor,
                current.cost_.value_ + tile->path_cost,                                    // Cost of current record to this node
                current.cost_.value_ + tile->path_cost + HeuristicEstimate(neighbor, end)); // Heuristic cost
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
            Set(true_costs[neighbor], pn.cost_.value_);
        }
    }

    return coords;
}

// Bfs until the comparator returns true.
void search::bfs(const types::GridPoint& start,
    std::function<bool(const world::Tile& tile)> to_expand) {
    if (!to_expand) return;
    std::queue<types::GridPoint> to_explore;
    // Used to avoid expanding nodes already explored.
    std::unordered_map<types::GridPoint, uint32_t, VectorHash> discovered;
    to_explore.push(start);
    // Start node is at depth 1.
    discovered[start] = 1;
    while (!to_explore.empty()) {
        const types::GridPoint& expand = to_explore.front();
        std::vector<types::GridPoint> neighbors;
        world::Neighbors(expand, neighbors);
        to_explore.pop();
        for (auto n : neighbors) {
            if (discovered.find(n) != discovered.end()) continue;
            auto t = world::GetTile(n);
            if (!t) continue;
            // Early out if the condition is meet.
            uint32_t current_depth = discovered[expand];
            if (to_expand(*t)) {
                to_explore.push(n);
                discovered[n] = current_depth + 1;
            }
        }
    }
}

bool search::NearestPathablePoint(const types::GridPoint& source,
    const std::vector<types::GridPoint>& tiles,
    types::GridPoint& out) {
    int max_distance = std::numeric_limits<int>::max();
    for (const auto& n : tiles) {
        auto t = world::GetTile(n);
        if (!t->CanPath()) continue;
        float d = DistanceSquared(source, n);
        if (d < max_distance) {
            max_distance = d;
            out = n;
        }
    }
    if (max_distance == std::numeric_limits<int>::max()) return false;
    return true;
}

//
//bool search::bfs_units(const sf::Vector3i& start,
//    uint32_t depth,
//    world_map::TileMap& tile_map,
//    std::function<bool(const Unit& unit)> comparator)
//{
//    auto find_units = [&comparator](const Tile& tile) {
//        if (tile.m_unit_ids.empty()) return false;
//        bool result = false;
//        for (auto id : tile.m_unit_ids) {
//            Unit* unit = unit::get_unit(id);
//            if (!unit) continue;
//            result |= comparator(*unit);
//        }
//        return result;
//    };
//    return bfs(start, depth, tile_map, find_units);
//}
//
//bool search::bfs_cities(const sf::Vector3i& start,
//    uint32_t depth,
//    world_map::TileMap& tile_map,
//    std::function<bool(const City& unit)> comparator) {
//    auto find_cities = [&comparator](const Tile& tile) {
//        if (!tile.m_city_id) return false;
//        City* c = city::get_city(tile.m_city_id);
//        if (!c) return false;
//        return comparator(*c);
//    };
//    return bfs(start, depth, tile_map, find_cities);
//}
//
//// Run bfs for each improvement to depth.
//bool search::bfs_improvements(const sf::Vector3i& start,
//    uint32_t depth,
//    world_map::TileMap& tile_map,
//    std::function<bool(const Improvement& unit)> comparator) {
//    auto find_improvements = [&comparator](const Tile& tile) {
//        if (tile.m_improvement_ids.empty()) return false;
//        bool result = false;
//        for (auto id : tile.m_improvement_ids) {
//            Improvement* i = improvement::get_improvement(id);
//            if (!i) continue;
//            result |= comparator(*i);
//        }
//        return result;
//    };
//    return bfs(start, depth, tile_map, find_improvements);
//}
//
//// Run bfs for each resource to depth.
//bool search::bfs_resources(const sf::Vector3i& start,
//    uint32_t depth,
//    world_map::TileMap& tile_map,
//    std::function<bool(const Resource& unit)> comparator) {
//    auto find_resources = [&comparator](const Tile& tile) {
//        if (tile.m_resource.m_type == fbs::RESOURCE_TYPE::UNKNOWN) return false;
//        bool result = false;
//        result |= comparator(tile.m_resource);
//        return result;
//    };
//    return bfs(start, depth, tile_map, find_resources);
//}
