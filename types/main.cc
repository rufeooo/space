#include <chrono> 
#include <iostream>

#include "common/vec3_generated.h"
#include "flatbuffers/flatbuffers.h"

struct Vec3 {
  float x_;
  float y_;
  float z_;
};

int main() {
  common::Vec3 vfp;
  Vec3 v;
  {
    // Test speed of setting flatbuffer vs struct
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 50000; ++i) {
      vfp.mutate_x(3.0f + float(i));
      vfp.mutate_y(2.3f + float(i));
      vfp.mutate_z(7.6f + float(i));
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(
          stop - start); 

    std::cout << "Flatbuffer: "
              << duration.count() << " microseconds" << std::endl;
  }
  {
    // Test speed of setting flatbuffer vs struct
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 50000; ++i) {
      v.x_ = (3.0f + float(i));
      v.y_ = (2.3f + float(i));
      v.z_ = (7.6f + float(i));
    }

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(
          stop - start); 
    
    std::cout << "Struct: "
              << duration.count() << " microseconds" << std::endl;
  }
  return 0;
}
