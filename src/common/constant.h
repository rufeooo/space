#pragma once

// Players in one game
#define MAX_PLAYER 2

// Invalid offset into an array type
enum { kInvalidIndex = -1 };

// Invalid unique identifier ('id') for assigning stable identification numbers
enum { kInvalidId = 0 };

enum { kMaxBlackboardItems = 16 };
enum { kMaxBlackboardValueSize = 64 };
enum { kMaxCameras = 16 };
enum { kMaxTags = 3 };
enum { kLocalCameraTag = 2 };

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
