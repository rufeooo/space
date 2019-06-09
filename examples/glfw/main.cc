#include <iostream>

#include "glad/gl.h"
#include "GLFW/glfw3.h"

int main() {
  if (!glfwInit()) {
    // init failed.
    std::cout << "glfwInit() failed" << std::endl; 
  }
  glfwTerminate();
  return 0;
}
