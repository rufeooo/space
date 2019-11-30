#include <iostream>

#include "math/vec.h"

struct Combined {
  uint64_t entity_id;
  math::Vec3f vec;
};

int main() {
  std::cout << sizeof(math::Vec3f) << std::endl;
  std::cout << sizeof(uint64_t) << std::endl;
  std::cout << sizeof(Combined) << std::endl;

  math::Vec3f data;

  data.x() = 0;
  data.y() = 1;
  data.z() = 2;

  std::cout << data.x() << std::endl;

  return 0;
}
