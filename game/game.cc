#include "game.h"

#include <chrono> 
#include <iostream>

namespace game {

void Game::Run(uint64_t loop_count) {
  std::chrono::microseconds accumulator(0);
  std::chrono::microseconds time(0);
  std::chrono::microseconds desired_delta_time(16666);
  uint64_t loops = 0;
  std::chrono::microseconds render_time;
  while (loop_count == 0 || loops < loop_count) {
    auto pre_render_time = std::chrono::high_resolution_clock::now();
    RunRenderer();
    auto post_render_time = std::chrono::high_resolution_clock::now();
    render_time +=
      std::chrono::duration_cast<std::chrono::microseconds>
        (post_render_time - pre_render_time);
    accumulator +=  render_time;
    while (accumulator >= desired_delta_time) {
      RunSystems();
      accumulator -= desired_delta_time;
      time += desired_delta_time;
      std::cout << time.count() << std::endl;
    }
    ++loops;
  }
}

}
