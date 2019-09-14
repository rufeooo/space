#include <iostream>

#include "components/rendering/line_component.h"
#include "components/rendering/triangle_component.h"
#include "renderer/glfw_renderer.h"

int main() {
  renderer::GLFWRenderer glfw_renderer;
  glfw_renderer.Start(480, 680, "Test");
  component::TriangleComponent triangle;
  glfw_renderer.Draw(triangle);
  component::LineComponent line;
  glfw_renderer.Draw(line);
  return 0;
}
