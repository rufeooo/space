#include <iostream>
#include "platform/macosx_window.mm"
#include "math/vec.h"

int
main(int argc, char** argv)
{
  window::Create("macosx is great", 1024, 768);
  while (!window::ShouldClose()) {
    window::PollEvents();

    math::Vec2f cursor = window::GetCursorPosition();
    std::cout << cursor.x << " " << cursor.y << std::endl;

    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    window::SwapBuffers(); 
  }

  return 0;
}
