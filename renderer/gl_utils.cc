#include "gl_utils.h"

namespace renderer {

std::string GLTypeToString(GLenum type) {
  switch (type) {
  case GL_BOOL: return "bool";
  case GL_INT: return "int";
  case GL_FLOAT: return "float";
  case GL_FLOAT_VEC2: return "vec2";
  case GL_FLOAT_VEC3: return "vec3";
  case GL_FLOAT_VEC4: return "vec4";
  case GL_FLOAT_MAT2: return "mat2";
  case GL_FLOAT_MAT3: return "mat3";
  case GL_FLOAT_MAT4: return "mat4";
  case GL_SAMPLER_2D: return "sampler2D";
  case GL_SAMPLER_3D: return "sampler3D";
  case GL_SAMPLER_CUBE: return "samplerCube";
  case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
  default: break;
  }
  return "other";
}

}
