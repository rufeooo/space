#pragma once

enum Key {
  KEYBOARD_NONE = 0x00,
  KEYBOARD_W = 0x01,
  KEYBOARD_A = 0x02,
  KEYBOARD_S = 0x04,
  KEYBOARD_D = 0x08,
  KEYBOARD_SPACE = 0x10,
};

struct InputComponent {
  uint8_t input_mask;
  uint8_t previous_input_mask;
};

  inline bool
IsKeyDown(uint8_t mask, Key key)
{
  return (mask & key) != 0;
}

  inline bool
IsKeyUp(uint8_t mask, Key key)
{
  return (mask & key) == 0;
}

  inline void
SetKeyDown(uint8_t& mask, Key key)
{
  mask |= key;
}

  inline void
SetKeyUp(uint8_t& mask, Key key)
{
  mask ^= key;
}

// namespace component
