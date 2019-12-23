#include <iostream>

#include "platform/platform.cc"

int
main()
{
  window::Create("Colored triangle", 640, 480);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << "Renderer: " << renderer << std::endl;
  std::cout << "Version: " << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  return 0;
}
