#include "search.h"
#include "world.h"
#include "tile.h"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>


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
