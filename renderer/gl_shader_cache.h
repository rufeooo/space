#pragma once

#include "shader_cache.h"

#include <glad/glad.h>
#include <unordered_map>
#include <unordered_set>

namespace renderer {

class GLShaderCache : public ShaderCache {
 public:
  bool CompileShader(
      const std::string& shader_name,
      ShaderType shader_type,
      const std::string& shader_src) override;

  bool GetShaderReference(
      const std::string& shader_name,
      uint32_t* shader_reference) override;

  bool LinkProgram(
      const std::string& program_name,
      const std::vector<std::string>& shader_names,
      bool validate_program) override;

  bool UseProgram(const std::string& program_name) override;

  bool GetProgramReference(
      const std::string& program_name,
      uint32_t* program_reference) override;

  // TODO This would probably be nice returned as a ProgramInfo struct
  // and stringified by a helper function in gl_utils.
  std::string GetProgramInfo(const std::string& program_name) override;

 private:
  std::unordered_map<std::string, uint32_t> shader_reference_map_;
  std::unordered_map<std::string, uint32_t> program_reference_map_;
  // A set of all shader sources that have been compiled.
  std::unordered_set<std::string> compiled_shader_sources_;
};

}  // renderer