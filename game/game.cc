#include "game.h"

#include <chrono> 
#include <iostream>

namespace game {

void Game::Run(uint64_t loop_count) {
  float accumulator = 0.0f;
  float t = 0.0f;
  static const float DT = 1.0f / 60.0f;
  std::chrono::time_point<std::chrono::high_resolution_clock>
    pre_render_time, post_render_time;
  uint64_t loops = 0;
  std::chrono::microseconds frame_time;
  while (loop_count == 0 || loops < loop_count) {
    pre_render_time = std::chrono::high_resolution_clock::now();
    RunRenderer();
    post_render_time = std::chrono::high_resolution_clock::now();
    frame_time = std::chrono::duration_cast<std::chrono::microseconds>(
        post_render_time - pre_render_time);
    std::cout << frame_time.count() << std::endl;
    while (accumulator >= DT) {
      RunSystems();
      accumulator -= DT;
      t += DT;
    }
  }
}

}
