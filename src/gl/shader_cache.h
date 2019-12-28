#pragma once

#include "shader_cache.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gl
{
enum ShaderType {
  VERTEX,
  FRAGMENT
};

class ShaderCache
{
 public:
  bool CompileShader(const std::string& shader_name, ShaderType shader_type,
                     const std::string& shader_src);

  bool GetShaderReference(const std::string& shader_name,
                          uint32_t* shader_reference);

  bool LinkProgram(const std::string& program_name,
                   const std::vector<std::string>& shader_names);

  bool UseProgram(const std::string& program_name);

  bool GetProgramReference(const std::string& program_name,
                           uint32_t* program_reference);

  // TODO This would probably be nice returned as a ProgramInfo struct
  // and stringified by a helper function in gl_utils.
  std::string GetProgramInfo(const std::string& program_name);

 private:
  std::unordered_map<std::string, uint32_t> shader_reference_map_;
  std::unordered_map<std::string, uint32_t> program_reference_map_;
  // A set of all shader sources that have been compiled.
  std::unordered_set<std::string> compiled_shader_sources_;
};

}  // namespace gl
