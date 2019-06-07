#include <iostream>
#include <chrono>

int main() {
  std::chrono::microseconds s;
  s = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now());
  std::cout << s.count() << std::endl;

  return 0;
}
