#include "game.h"

#include <iostream>
#include <thread>

namespace game {

inline std::chrono::milliseconds NowMS() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
}

bool Game::Run(uint64_t loop_count) {
  if (!Initialize()) {
    return false;
  }
  auto previous = NowMS();
  std::chrono::milliseconds lag(0);
  game_updates_ = 0;
  game_time_ = std::chrono::milliseconds(0);
  std::chrono::milliseconds current, elapsed, end_loop;
  while (loop_count == 0 || game_updates_ < loop_count) {
    if (end_) return true;
    current = NowMS();
    elapsed = current - previous;
    if (!paused_) lag += elapsed;
    ProcessInput();
    real_time_ += elapsed;
    while (!paused_ && lag >= ms_per_update_) {
      Update();
      lag -= ms_per_update_;
      game_time_ += ms_per_update_;
      ++game_updates_;
    }
    if (!Render()) return true; // Returns ??
    end_loop = NowMS();
    previous = current;
    auto ms = end_loop - current;
    // sleep s.t. we only do min_ms_per_frame_.
    if (sleep_on_loop_end_ && ms < min_ms_per_frame_) {
      auto sleep_time = min_ms_per_frame_ - ms;
      std::this_thread::sleep_for(sleep_time);
    }
    ms_per_frame_ = NowMS() - current;
  }
  return true;
}

void Game::Pause() {
  paused_ = true;
}

void Game::Resume() {
  paused_ = false;
}

void Game::End() {
  end_ = true;
}

}
