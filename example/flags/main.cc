#include <iostream>

#include <gflags/gflags.h>

DEFINE_string(message, "DEFAULT", "Helpful description");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::cout << FLAGS_message << std::endl;
  return 0;
}
