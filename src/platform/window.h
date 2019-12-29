#pragma once

#include "math/vec.h"

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
//
// Notably this module creates a single window and does not intend
// on supporting making multiple.

// GL types.
typedef size_t GLsizeiptr;
typedef char GLchar;
typedef float GLfloat;
typedef unsigned int GLenum;
typedef unsigned int GLuint;

enum PlatformEventType {
  NOT_IMPLEMENTED,  // Event does not have translation implemented yet.
  MOUSE_DOWN,
  MOUSE_UP,
  KEY_DOWN,
  KEY_UP,
};

enum PlatformButton {
  BUTTON_UNKNOWN,
  BUTTON_LEFT = 1,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
};

struct PlatformEvent {
  // Type of event.
  PlatformEventType type;
  // Screen space the event took place in.
  math::Vec2f position;
  // Event Detail
  union {
    char key;
    PlatformButton button;
  };
};

namespace window
{
int Create(const char* name, int width, int height);

// Returns true if an event existed. False otherwise.
// Fully poll this queue at the top of each game loop.
bool PollEvent(PlatformEvent* event);

void SwapBuffers();

bool ShouldClose();

math::Vec2f GetWindowSize();

math::Vec2f GetCursorPosition();
}  // namespace window
