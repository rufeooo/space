#include "utils.h"

#include <alloca.h>

namespace gl
{
const char*
GLTypeToString(int type)
{
  switch (type) {
    case GL_BOOL:
      return "bool";
    case GL_INT:
      return "int";
    case GL_FLOAT:
      return "float";
    case GL_FLOAT_VEC2:
      return "vec2";
    case GL_FLOAT_VEC3:
      return "vec3";
    case GL_FLOAT_VEC4:
      return "vec4";
    case GL_FLOAT_MAT2:
      return "mat2";
    case GL_FLOAT_MAT3:
      return "mat3";
    case GL_FLOAT_MAT4:
      return "mat4";
    case GL_SAMPLER_2D:
      return "sampler2D";
    case GL_SAMPLER_3D:
      return "sampler3D";
    case GL_SAMPLER_CUBE:
      return "samplerCube";
    case GL_SAMPLER_2D_SHADOW:
      return "sampler2DShadow";
    default:
      break;
  }
  return "other";
}

uint32_t
CreateTriangleVAO(
    )
{
  GLfloat points[] = {
      0.0f, 0.1f, 0.0f, 0.1f, -0.1f, 0.0f, -0.1f, -0.1f, 0.0f,
  };
  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return vao;
}

uint32_t
CreateGeometryVAO(int len, GLfloat verts[len])
{
  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, len * sizeof(GLfloat), &verts[0],
               GL_STATIC_DRAW);
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return vao;
}

// Creates a vbo for a vert list of 2d vectors.
uint32_t
CreateGeometryVAO(int len, math::Vec2f verts[len])
{
  // Might be worth a malloc if we ever create complicated geometry.
  GLfloat* vert_list = (GLfloat*)alloca(len * 3 * sizeof(GLfloat));
  int k = 0;
  for (int i = 0; i < len; ++i) {
    vert_list[k++] = verts[i].x;
    vert_list[k++] = verts[i].y;
    vert_list[k++] = 0.f;
  }
  return CreateGeometryVAO(len * 3, vert_list);
}

}  // namespace renderer
