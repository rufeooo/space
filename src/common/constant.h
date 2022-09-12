// clang-format off
#pragma once

// Players in one game
#define MAX_PLAYER 2

// Invalid offset into an array type 
// Equivalent to memset(0xff) for up to 64 bits
#define kInvalidIndex ((uint64_t){-1})

// Invalid unique identifier ('id') for assigning stable identification numbers
// Equivalent to memset(0x00) for up to 64 bits
// Allows for boolean tests: (id ? "exists: "not-exists")
#define kInvalidId ((uint64_t){0})

#define kMaxBlackboardItems 16
#define kMaxBlackboardValueSize 64

enum PlatformEventType {
  NOT_IMPLEMENTED,  // Event does not have translation implemented yet.
  MOUSE_DOWN,
  MOUSE_UP,
  MOUSE_WHEEL,
  KEY_DOWN,
  KEY_UP,
  MOUSE_POSITION,
};

enum PlatformButton {
  BUTTON_UNKNOWN,
  BUTTON_LEFT = 1,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
};
