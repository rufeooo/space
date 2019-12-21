#include <iostream>
#include "platform/win32_window.cc"
#include "math/vec.h"

int
main(int argc, char** argv)
{
  window::Create("Windows is great", 1024, 768);
  while (!window::ShouldClose()) {
    window::PollEvents();

    if (window::HasInput(KEY_W)) {
      std::cout << "W DOWN" << std::endl;
    }

    if (window::HasInput(KEY_A)) {
      std::cout << "A DOWN" << std::endl;
    }

    if (window::HasInput(KEY_S)) {
      std::cout << "S DOWN" << std::endl;
    }

    if (window::HasInput(KEY_D)) {
      math::Vec2f size = window::GetWindowSize();
      std::cout << size.x << " " << size.y << std::endl;
    }

    if (window::HasInput(MOUSE_LEFT_CLICK)) {
      math::Vec2f cursor = window::GetCursorPosition();
      std::cout << cursor.x << " " << cursor.y << std::endl;
    }


    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    window::SwapBuffers(); 
  }

  return 0;
}
