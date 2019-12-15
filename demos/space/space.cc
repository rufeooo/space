#include <iostream>

#include "game/game.h"
#include "renderer/gl_utils.h"

namespace {

struct OpenGL {
  GLFWwindow* glfw = nullptr;
};

static OpenGL kOpenGL;

bool Initialize() {
  kOpenGL.glfw = renderer::InitGLAndCreateWindow(800, 800, "Space");
  if (!kOpenGL.glfw) {
    std::cout << "Unable to start GL and create window."
              << std::endl;
    return false;
  }
  return true;
}

bool ProcessInput() {
  glfwPollEvents();
  return true;
}

void HandleEvent(game::Event event) {
}

bool UpdateGame() {
  return true;
}

bool RenderGame() {
  glfwSwapBuffers(kOpenGL.glfw);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return !glfwWindowShouldClose(kOpenGL.glfw);
}

void OnEnd() {
}

}

int main(int argc, char** argv) {
  game::Setup(&Initialize,
              &ProcessInput,
              &HandleEvent,
              &UpdateGame,
              &RenderGame,
              &OnEnd);

  if (!game::Run()) {
    std::cerr << "Encountered error running spacey game..." << std::endl;
  }

  return 0;

}
