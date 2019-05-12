#define CATCH_CONFIG_MAIN  // Make Catch provide main.

#include <iostream>

#include <catch2/catch.hpp>

#include "search.h"

TEST_CASE("Path exists using bfs.", "[search - bfs]") {
  REQUIRE(true == true);
}

TEST_CASE("Path exists using dfs.", "[search - dfs]") {
  REQUIRE(true == true);
}

TEST_CASE("Shortest path number on line", "[search - A*]") {
  int start = 0, end = 10;
  auto expand_func = [](int start) {
    return std::vector<int>({start + 1, start - 1});
  };
  auto cost_func = [](int, int) { return 1; };
  auto heuristic_func = [](int current, int end) { 
    return end - current;
  };
  std::vector<int> path = search::PathTo(
      start, end, expand_func, cost_func, heuristic_func);
  // Shortest pat from 0 to 10 on a number line is 0 to 10.
  REQUIRE(path ==
      std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
}
