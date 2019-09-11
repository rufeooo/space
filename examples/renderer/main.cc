#include <iostream>

#include "components/rendering/triangle_component.h"
#include "renderer/renderer.h"

int main() {
  renderer::Renderer glfw_renderer;
  glfw_renderer.Start(480, 680, "Test");
  component::TriangleComponent triangle;
  glfw_renderer.Draw(triangle);
  component::LineComponent line;
  glfw_renderer.Draw(line);
  return 0;
}
