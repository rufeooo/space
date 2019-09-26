#pragma once

#include "renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace renderer {

class GLFWRenderer : public Renderer {
 public:
  bool Start(
    int window_width, int window_height,
    const std::string& window_title) override;
  bool HasStopped() const override;
  bool SwapBuffers() override;

  GLFWwindow* window() { return window_; }
 private:
  GLFWwindow* window_;
};

}  // renderer
