#include <iostream>

enum Key {
  KEYBOARD_NONE  = 0x00,
  KEYBOARD_W     = 0x01,
  KEYBOARD_A     = 0x02,
  KEYBOARD_S     = 0x04,
  KEYBOARD_D     = 0x08,
  KEYBOARD_SPACE = 0x10,
};

bool IsSet(int16_t value, Key key) {
  return (value & key) != 0;
}

int main() {
  int16_t a = 0;
  a |= KEYBOARD_SPACE;
  int16_t b = KEYBOARD_A | KEYBOARD_D;
  std::cout << "W: " << IsSet(b, KEYBOARD_W) << std::endl;
  std::cout << "A: " << IsSet(b, KEYBOARD_A) << std::endl;
  std::cout << "S: " << IsSet(b, KEYBOARD_S) << std::endl;
  std::cout << "D: " << IsSet(b, KEYBOARD_D) << std::endl;
  std::cout << "SPACE: " << IsSet(b, KEYBOARD_SPACE) << std::endl;
  return 0;
}
