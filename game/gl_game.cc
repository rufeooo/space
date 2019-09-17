#include "gl_game.h"

#include "renderer/glfw_renderer.h"

namespace game {

GLGame::~GLGame() {
}

bool GLGame::Initialize() {
  if (!glfw_renderer_.Start(
      window_width_, window_height_, "GLGame")) {
    return false;
  }
  return true;
}

bool GLGame::ProcessInput() {
  return true;
}

bool GLGame::Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwPollEvents();
  glfw_renderer_.SwapBuffers();
  return !glfw_renderer_.HasStopped();
}

}  // game
