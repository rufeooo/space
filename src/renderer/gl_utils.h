#pragma once

#include <string>
#include <vector>

#include "math/vec.h"

namespace renderer
{
void InitGLAndCreateWindow(int window_width, int window_height, const char* title);

// Given a GLenum returns a stringified version of it. For example -
// GLTypeToString(GL_BOOL) = "bool"
// GLTypeToString(GL_INT) = "int"
// GLTypeToString(GL_FLOAT) = "float"
std::string GLTypeToString(int type);

// Creates vbo for a triangle and binds a vao with those points.
// Returns resulting  vao. A user should call those once and cache
// the id returned from it.
uint32_t CreateTriangleVAO();

// Creates a vbo for a vert list and binds / returns a vao.
uint32_t CreateGeometryVAO(const std::vector<float>& verts);

// Creates a vbo for a vert list of 2d vectors.
uint32_t CreateGeometryVAO(const std::vector<math::Vec2f>& verts);

}  // namespace renderer
