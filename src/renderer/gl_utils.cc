#include "gl_utils.h"

#include <iostream>

namespace renderer
{

void InitGLAndCreateWindow(int window_width, int window_height, const char* title)
{
  window::Create(title, window_width, window_height);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  // Should these be enabled?
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  // glFrontFace(GL_CW);
}

std::string
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
CreateTriangleVAO()
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
CreateGeometryVAO(const std::vector<GLfloat>& verts)
{
  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(),
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
CreateGeometryVAO(const std::vector<math::Vec2f>& verts)
{
  std::vector<GLfloat> vert_list;
  vert_list.reserve(verts.size() * 3);
  for (const auto& v : verts) {
    vert_list.push_back(v.x);
    vert_list.push_back(v.y);
    vert_list.push_back(0.f);  // TODO: Maybe consider removing this.
  }
  return CreateGeometryVAO(vert_list);
}

}  // namespace renderer
