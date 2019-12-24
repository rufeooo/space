#include "platform/platform.cc"

#include <iostream>

int 
main(int argc, char** argv)
{
  std::cout << platform::now_ns() << std::endl;
  return 0;
}
