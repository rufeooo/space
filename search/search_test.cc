#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"

#include "search.h"

// Graph for testing purposes. Does not handle many edge cases, such
// as duplicate edge insertion, and should not be used for anything
// other than these tests without significant modification.
class SimpleGraph {
 public:
  void Add(char from, char to, uint32_t weight=0) {
    Edge e;
    e.from_ = from;
    e.to_ = to;
    e.weight_ = weight;
    edges_[from].push_back(e);
  }

  std::vector<char> Neighbors(char node) const {
    std::vector<char> n;
    const auto& found = edges_.find(node);
    if (found == edges_.end()) return n;
    for (const auto& e : found->second) {
      n.push_back(e.to_);
    }
    return n;
  }

  // This function is wrong because it implies a path is possible
  // between a and b even if it's not.
  uint32_t Weight(char from, char to) const {
    const auto& found = edges_.find(from);
    if (found == edges_.end()) return UINT32_MAX;
    for (const auto& e : found->second) {
      if (e.to_ == to) return e.weight_;
    }
    return UINT32_MAX;
  }

  void Clear() {
    edges_.clear();
  }

 private:
  struct Edge {
    char from_;
    char to_;
    uint32_t weight_;
  };

  std::unordered_map<char, std::vector<Edge>> edges_;
};

TEST(SearchTest, PathExistsBFS) {
  //   I   K
  //   B   J
  // D A C G H
  //   E
  //   F
  SimpleGraph sg;
  sg.Add('A', 'B');
  sg.Add('A', 'C');
  sg.Add('A', 'D');
  sg.Add('A', 'E');
  sg.Add('E', 'F');
  sg.Add('C', 'G');
  sg.Add('G', 'H');
  sg.Add('G', 'J');
  sg.Add('J', 'K');
  sg.Add('B', 'I');
  // Find H
  {
    std::unordered_set<char> explored;
    auto expand = [&explored, &sg](char from) {
      explored.insert(from);
      if (from == 'H') {
        return std::vector<char>();
      }
      return sg.Neighbors(from);
    };
    search::BreadthFirst('A', 3, expand);
    ASSERT_TRUE(explored.find('H') != explored.end());
    explored.clear();
    search::BreadthFirst('A', 2, expand);
    ASSERT_TRUE(explored.find('H') == explored.end());
  }

  {
    std::unordered_set<char> explored;
    auto expand = [&explored, &sg](char from) {
      explored.insert(from);
      if (from == 'K') {
        return std::vector<char>();
      }
      return sg.Neighbors(from);
    };
    search::BreadthFirst('A', 4, expand);
    ASSERT_TRUE(explored.find('K') != explored.end());
    explored.clear();
    search::BreadthFirst('A', 3, expand);
    ASSERT_TRUE(explored.find('K') == explored.end());
  }
}

TEST(SearchTest, PathExistsAStar) {
  {
    // Example graph found at -
    // https://en.wikipedia.org/wiki/A*_search_algorithm
    SimpleGraph sg;
    auto expand_func = [&sg](char start) {
      return sg.Neighbors(start);
    };
    auto cost_func = [&sg](char a, char b) {
      return sg.Weight(a, b);
    };
    sg.Add('0', 'a', 15);
    sg.Add('0', 'd', 20);
    sg.Add('a', 'b', 20);
    sg.Add('b', 'c', 30);
    sg.Add('d', 'e', 30);
    // Change this weight to 10 to see path change.
    sg.Add('c', 'f', 40);
    sg.Add('e', 'f', 20);
    std::vector<char> path = search::PathTo(
      '0', 'f', expand_func, cost_func, cost_func);
    ASSERT_TRUE(path == std::vector<char>({'0', 'd', 'e', 'f'}));
  }

  {
    SimpleGraph sg;
    auto expand_func = [&sg](char start) {
      return sg.Neighbors(start);
    };
    auto cost_func = [&sg](char a, char b) {
      return sg.Weight(a, b);
    };
    sg.Add('0', 'a', 15);
    sg.Add('0', 'd', 20);
    sg.Add('a', 'b', 20);
    sg.Add('b', 'c', 30);
    sg.Add('d', 'e', 30);
    sg.Add('c', 'f', 10);
    sg.Add('e', 'f', 20);
    std::vector<char> path = search::PathTo(
      '0', 'f', expand_func, cost_func, cost_func);
    ASSERT_EQ(path, std::vector<char>({'0', 'a', 'b', 'c', 'f'}));
  }
}

TEST(SearchTest, ShortestPathNumberOfLines) {
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
  ASSERT_TRUE(path ==
      std::vector<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
