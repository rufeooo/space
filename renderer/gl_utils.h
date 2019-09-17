#pragma once

#include <string>
#include <glad/glad.h>

namespace renderer {

// Given a GLenum returns a stringified version of it. For example -
// GLTypeToString(GL_BOOL) = "bool"
// GLTypeToString(GL_INT) = "int"
// GLTypeToString(GL_FLOAT) = "float"
std::string GLTypeToString(GLenum type);

// Creates vbo for a triangle and binds a vao with those points.
// Returns resulting  vao. A user should call those once and cache
// the id returned from it.
uint32_t CreateTriangleVAO();

}
