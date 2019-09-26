#pragma once

#include <string>

namespace renderer {

class Renderer {
 public:
  Renderer() = default;
  virtual ~Renderer() = default;
  virtual bool Start(
    int window_width, int window_height,
    const std::string& window_title) = 0;
  virtual bool SwapBuffers() = 0;
  virtual bool HasStopped() const = 0;
};

}  // renderer
