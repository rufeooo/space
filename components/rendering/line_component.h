#pragma once

#include "math/vec.h"

namespace component {

struct LineComponent {
  // ID to the vertices bound to OpenGL.
  uint32_t vao_reference;
  // ID to the compiled / linked OpenGL shader program.
  uint32_t program_reference;
};

}
