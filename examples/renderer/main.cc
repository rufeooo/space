#include <iostream>

#include "renderer/renderer.h"

int main() {
  renderer::Renderer glfw_renderer;
  glfw_renderer.Start(480, 680, "Test");
  return 0;
}
