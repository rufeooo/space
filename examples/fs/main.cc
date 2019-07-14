#include <iostream>

#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>

int main() {
  using namespace cppfs;
  FileHandle dir = fs::open("./");
  if (dir.isDirectory()) {
    for (const auto& f : dir) {
      std::cout << f << std::endl;
    }
  }

  return 0;
}
