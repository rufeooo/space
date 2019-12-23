#include <iostream>
#include "platform/win32_window.cc"
#include "math/vec.h"

int
main(int argc, char** argv)
{
  window::Create("Windows is great", 1024, 768);
  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case MOUSE_LEFT_DOWN: {
          std::cout << event.position.x << " " << event.position.y << std::endl;
        } break;
        default: break;
      }
    }

    glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    window::SwapBuffers(); 
  }

  return 0;
}
