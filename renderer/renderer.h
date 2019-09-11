#pragma once

#include <string>

#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"

namespace renderer {

class Renderer {
 public:
  Renderer() = default;
  bool Start(
    int window_width, int window_height,
    const std::string& window_title);
  void Draw(const component::LineComponent& component);
  void Draw(const component::TriangleComponent& component);
};

}  // renderer
