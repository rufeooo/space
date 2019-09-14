#pragma once

#include <string>
#include <vector>

namespace renderer {

enum class ShaderType {
  VERTEX,
  FRAGMENT
};

class ShaderCache {
 public:
  virtual bool CompileShader(
      const std::string& shader_name,
      ShaderType shader_type,
      const std::string& shader_src) = 0;

  virtual bool GetShaderReference(
      const std::string& shader_name,
      uint32_t* shader_reference) = 0;

  virtual bool LinkShaders(
      const std::string& program_name,
      const std::vector<std::string>& shader_names) = 0;

  virtual bool UseProgram(const std::string& program_name) = 0;

  virtual bool GetProgramReference(
      const std::string& program_name,
      uint32_t* program_reference) = 0;
};

}
