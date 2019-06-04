#include "game.h"

#include <chrono> 
#include <iostream>

namespace game {

void Game::Run(uint64_t loop_count) {
  using Seconds = std::chrono::duration<double>;
  Seconds accumulator;
  Seconds t = Seconds(0.0f);
  static const Seconds DT = Seconds(1.0f / 60.0f);
  Seconds pre_render_time, post_render_time;
  uint64_t loops = 0;
  while (loop_count == 0 || loops < loop_count) {
    auto pre_render_time = std::chrono::system_clock::now();
    RunRenderer();
    auto post_render_time = std::chrono::system_clock::now();
    accumulator += (post_render_time - pre_render_time); 
    std::cout << accumulator.count() << std::endl;
    while (accumulator >= DT) {
      RunSystems();
      accumulator -= DT;
      t += DT;
    }
    ++loops;
  }
}

}
