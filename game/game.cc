#include "game.h"

#include <iostream>

namespace game {

inline std::chrono::microseconds Now() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}

void Game::Run(uint64_t loop_count) {
  Initialize();
  std::chrono::microseconds previous = Now();
  std::chrono::microseconds lag(0);
  game_updates_ = 0;
  game_time_ = std::chrono::microseconds(0);
  while (loop_count == 0 || game_updates_ < loop_count) {
    std::chrono::microseconds current = Now();
    std::chrono::microseconds elapsed = current - previous;
    if (!paused_) lag += elapsed;
    ProcessInput();
    real_time_ += elapsed;
    if (!paused_ && lag >= microseconds_per_update_) {
      Update();
      lag -= microseconds_per_update_;
      game_time_ += microseconds_per_update_;
      ++game_updates_;
    }
    Render();
  }
}

void Game::Pause() {
  paused_ = true;
}

void Game::Resume() {
  paused_ = false;
}

}
