#include <iostream>
#include "common/vec3_generated.h"
#include "flatbuffers/flatbuffers.h"

int main() {
  common::Vec3 v;
  v.mutate_x(3.0f);
  v.mutate_y(2.3f);
  v.mutate_z(7.6f);
  std::cout << v.x() << "," << v.y() << "," << v.z() << std::endl;
  return 0;
}
