#pragma once

#include <string>

#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"

namespace renderer {

class Renderer {
 public:
  Renderer() = default;
  virtual ~Renderer() = default;
  virtual bool Start(
    int window_width, int window_height,
    const std::string& window_title) = 0;

  virtual void Draw(const component::LineComponent& component) = 0;
  virtual void Draw(const component::TriangleComponent& component) = 0;
};

}  // renderer
