#pragma once

// Cross-platform window / opengl context abstraction. The purpose
// of these functions are to setup a window on the screen and setup
// the opengl context.
// 
// Notably this module creates a single window and does not intend
// on supporting making multiple.

namespace window
{
void Create(const char* name, int width, int height);

void PollEvents();

void SwapBuffers();

bool ShouldClose();
}  // namespace window
