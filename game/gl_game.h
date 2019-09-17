#pragma once

#include "game.h"

#include "renderer/glfw_renderer.h"

namespace game {

class GLGame : public Game {
 public:
  GLGame(int width, int height) :
    window_width_(width), window_height_(height) {}

  ~GLGame() override;

  bool Initialize() override;
  bool ProcessInput() override;
  bool Render() override;

 protected:
  renderer::GLFWRenderer glfw_renderer_;
  int window_width_;
  int window_height_;
};

}
