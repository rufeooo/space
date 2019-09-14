#pragma once

#include "renderer.h"

namespace renderer {

class GLFWRenderer : public Renderer {
 public:
  bool Start(
    int window_width, int window_height,
    const std::string& window_title) override;

  void Draw(const component::LineComponent& component) override;
  void Draw(const component::TriangleComponent& component) override;
};

}  // renderer
