#include "gl_shader_cache.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <glad/glad.h>

#include "gl_utils.h"

namespace renderer {

namespace {

GLenum ToGLEnum(ShaderType shader_type) {
  switch (shader_type) {
    case ShaderType::VERTEX:
      return GL_VERTEX_SHADER;
    case ShaderType::FRAGMENT:
      return GL_FRAGMENT_SHADER;
  }
  return -1;
}

std::string GetShaderInfoLog(uint32_t shader_reference) {
  constexpr int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetShaderInfoLog(
      shader_reference, log_length, &actual_length, log);
  return "Shader Log for reference: " +
         std::to_string(shader_reference) +
         " log: " + log;
}

std::string GetProgramInfoLog(uint32_t program_reference) {
  constexpr int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetProgramInfoLog(
      program_reference, log_length, &actual_length, log);
  return "Program Log for reference: " +
         std::to_string(program_reference) +
         " log: " + log;
}

}  // anonymous

bool GLShaderCache::CompileShader(
       const std::string& shader_name,
       ShaderType shader_type,
       const std::string& shader_src) {
  // Don't compile a shader with the same name or the original one will
  // be orphaned.
  if (shader_reference_map_.find(shader_name) !=
      shader_reference_map_.end()) {
    return false;
  }
  // Trying to compile is likely an accident from the user. It would
  // be good to tell them that.
  if (compiled_shader_sources_.find(shader_src) !=
      compiled_shader_sources_.end()) {
    return false;
  }
  GLenum gl_shader_type = ToGLEnum(shader_type);
  if (gl_shader_type == GLenum(-1)) {
    return false;
  }
  GLuint shader_reference = glCreateShader(gl_shader_type);
  const char* c_str = shader_src.c_str();
  glShaderSource(shader_reference, 1, &c_str, NULL);
  glCompileShader(shader_reference);
  int params = -1;
  glGetShaderiv(shader_reference, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE) {
    // TODO: switch to LOG
    std::cout << GetShaderInfoLog(shader_reference) << std::endl;
    return false;
  }
  shader_reference_map_[shader_name] = shader_reference;
  compiled_shader_sources_.insert(shader_src);
  return true;
}

bool GLShaderCache::GetShaderReference(
    const std::string& shader_name,
    uint32_t* shader_reference) {
  auto found = shader_reference_map_.find(shader_name);
  if (found == shader_reference_map_.end()) {
    return false;
  }
  *shader_reference = found->second;
  return true;
}

bool GLShaderCache::LinkProgram(
    const std::string& program_name,
    const std::vector<std::string>& shader_names) {
  if (program_reference_map_.find(program_name) !=
      program_reference_map_.end()) {
    return false;
  }
  GLuint program_reference = glCreateProgram();
  for (const auto& shader : shader_names) {
    uint32_t shader_reference;
    if (!GetShaderReference(shader, &shader_reference)) {
      return false;
    }
    glAttachShader(program_reference, shader_reference);
  }
  glLinkProgram(program_reference);
  int params = -1;
  glGetProgramiv(program_reference, GL_LINK_STATUS, &params);
  if (params != GL_TRUE) {
    // TODO: switch to LOG
    std::cout << GetProgramInfoLog(program_reference) << std::endl;
    return false;
  }
  program_reference_map_[program_name] = program_reference;
  return true;
}

bool GLShaderCache::UseProgram(const std::string& program_name) {
  auto found = program_reference_map_.find(program_name);
  if (found == program_reference_map_.end()) {
    return false;
  }
  glUseProgram(found->second);
  return true;
}

bool GLShaderCache::GetProgramReference(
    const std::string& program_name,
    uint32_t* program_reference) {
  auto found = program_reference_map_.find(program_name);
  if (found == program_reference_map_.end()) {
    return false;
  }
  *program_reference = found->second;
  return true;
}

std::string GLShaderCache::GetProgramInfo(
    const std::string& program_name) {
  uint32_t program_reference;
  if (!GetProgramReference(program_name, &program_reference)) {
    return "Program does not exist.";
  }
  std::stringstream ss;
  ss << "Shader program: " << program_name
     << " Reference: " << program_reference << std::endl;
  int params = -1;
  glGetProgramiv(program_reference, GL_LINK_STATUS, &params);
  ss << "GL_LINK_STATUS = " << params << std::endl;
  glGetProgramiv(program_reference, GL_ATTACHED_SHADERS, &params);
  ss << "GL_ATTACHED_SHADERS = " << params << std::endl;
  glGetProgramiv (program_reference, GL_ACTIVE_ATTRIBUTES, &params);
  ss << "GL_ACTIVE_ATTRIBUTES = " << params << std::endl;
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveAttrib(
        program_reference, i, max_length, &actual_length, &size,
        &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        std::sprintf(long_name, "%s[%i]", name, j);
        int location = glGetAttribLocation(
            program_reference, long_name);
        ss << i << ") type: " << GLTypeToString(type)
           << " name: " << long_name
           << " location: " << location << std::endl;
      }
    } else {
      int location = glGetAttribLocation(program_reference, name);
      ss << i << ") type: " << GLTypeToString(type)
         << " name: " << name
         << " location: " << location << std::endl;
    }
  }
  glGetProgramiv(program_reference, GL_ACTIVE_UNIFORMS, &params);
  ss << "GL_ACTIVE_UNIFORMS = " << params << std::endl;
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveUniform(
        program_reference, i, max_length, &actual_length, &size,
        &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
       char long_name[64];
       std::sprintf(long_name, "%s[%i]", name, j);
       int location = glGetUniformLocation(
           program_reference, long_name);
       ss << i << ") type: " << GLTypeToString(type)
          << " name: " << long_name
          << " location: " << location << std::endl;
      }
    } else {
      int location = glGetUniformLocation(program_reference, name);
      ss << i << ") type: " << GLTypeToString(type)
         << " name: " << name
         << " location: " << location << std::endl;
    }
  }
  return ss.str();
}

}  // renderer
