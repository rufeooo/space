#pragma once

#include "math/vec.h"
#include "platform/platform.cc"

namespace gl
{
// Given a GLenum returns a const char* version of it. For example -
// GLTypeToString(GL_BOOL) = "bool"
// GLTypeToString(GL_INT) = "int"
// GLTypeToString(GL_FLOAT) = "float"
const char* GLTypeToString(int type);

// Creates a vbo for a vert list and binds / returns a vao.
uint32_t CreateGeometryVAO(int len, GLfloat* verts);

}  // namespace gl
