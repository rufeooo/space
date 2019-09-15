#include "gl_shader_cache.h"

#include <iostream>
#include <glad/glad.h>

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
  int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetShaderInfoLog(
      shader_reference, log_length, &actual_length, log);
  return "Shader Log for reference: " +
         std::to_string(shader_reference) +
         " log: " +
         log;
}

std::string GetProgramInfoLog(uint32_t program_reference) {
  int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetProgramInfoLog(
      program_reference, log_length, &actual_length, log);
  return "Program Log for reference: " +
         std::to_string(program_reference) +
         " log: " +
         log;
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

}  // renderer
