#include "gl_game.h"

#include "renderer/glfw_renderer.h"

namespace game {

GLGame::~GLGame() {
}

bool GLGame::Initialize() {
  return glfw_renderer_.Start(
      window_width_, window_height_, "GLGame");
}

bool GLGame::ProcessInput() {
  return true;
}

bool GLGame::Render() {
    return !glfw_renderer_.HasStopped();
}

}  // game
