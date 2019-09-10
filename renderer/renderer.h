#pragma once

#include <string>

namespace renderer {

class Renderer {
 public:
  Renderer() = default;
  bool Start(
    int window_width, int window_height,
    const std::string& window_title);
};

}  // renderer
