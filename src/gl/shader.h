#pragma once

#include <stdarg.h>
#include <string>

#include "platform/platform.cc"

namespace gl
{

// Helper for compiling shaders that prints out errors if any exist.
bool CompileShader(GLenum shader_type, const GLchar* const* src, GLuint* id);

// Link shader and check for errors if any exist.
bool LinkShaders(GLuint* id, int n, ...);

// Given a program id returns -
// LINK_STATUS, ATTACHED_SHADERS, ACTIVE_ATTRIBUTES, ACTIVE_UNIFORMS
// as a string. Useful for debugging.
std::string GetProgramInfo(GLuint id);

}  // namespace gl
