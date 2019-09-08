#pragma once

#include <chrono> 
#include <cstdint>

namespace game {

class Game {
 public:
  virtual ~Game() = default;
  virtual void Initialize() {};
  virtual void ProcessInput() = 0;
  virtual void Update() = 0;
  virtual void Render() = 0;

  // Setting to 0 runs indefinitely.
  void Run(uint64_t loop_count=0);
  void Pause();
  void Resume();
  void End();

  uint64_t game_updates() const { return game_updates_; }
  std::chrono::microseconds game_time() const { return game_time_; }
 private:
  std::chrono::microseconds microseconds_per_update_ = 
    std::chrono::microseconds(10000);
  std::chrono::microseconds game_time_;
  std::chrono::microseconds real_time_;
  bool paused_ = false;
  bool end_ = false;
  // Number of times the game has been updated.
  uint64_t game_updates_;
};

}