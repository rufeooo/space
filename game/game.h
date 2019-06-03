#pragma once

#include <cstdint>

namespace game {

class Game {
 public:
  virtual void RunSystems() = 0;
  virtual void RunRenderer() = 0;

  // Setting to 0 runs indefinitely.
  void Run(uint64_t loop_count=0);
};

}
