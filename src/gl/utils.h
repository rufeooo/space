#pragma once

#include "math/vec.h"
#include "platform/window.h"

namespace gl
{
// Given a GLenum returns a const char* version of it. For example -
// GLTypeToString(GL_BOOL) = "bool"
// GLTypeToString(GL_INT) = "int"
// GLTypeToString(GL_FLOAT) = "float"
const char* GLTypeToString(int type);

// Creates vbo for a triangle and binds a vao with those points.
// Returns resulting  vao. A user should call those once and cache
// the id returned from it.
uint32_t CreateTriangleVAO();

// Creates a vbo for a vert list and binds / returns a vao.
uint32_t CreateGeometryVAO(int len, GLfloat verts[len]);

// Creates a vbo for a vert list of 2d vectors.
uint32_t CreateGeometryVAO(int len, math::Vec2f verts[len]);

}  // namespace gl
