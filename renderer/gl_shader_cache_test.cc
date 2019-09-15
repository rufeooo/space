#include "gl_shader_cache.h"
#include "glfw_renderer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace renderer {

constexpr const char* kValidVertexShaderSrc =
    "#version 410\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";

constexpr const char* kValidVertexShaderName = "valid_vertex_shader";

constexpr const char* kInvalidVertexShaderSrc =
    "#version 410\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0)" // Syntax error - missing ;
    "}";

constexpr const char* kInvalidVertexShaderName
    = "invalid_vertex_shader";

constexpr const char* kValidFragmentShaderSrc =
    "#version 410\n"
    "out vec4 frag_colour;"
    "void main() {"
    " frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
    "}";

constexpr const char* kValidFragmentShaderName
    = "valid_fragment_shader";

constexpr const char* kValidProgramName
    = "valid_fragment_shader";

class GLShaderCacheTest : public ::testing::Test {
 protected:
  GLShaderCacheTest() {
    // Required to initialize OpenGL context and glad.
    renderer_.Start(10, 10, "fake");
  }

  GLFWRenderer renderer_;
};

TEST_F(GLShaderCacheTest, HappyPathShaderCompilation) {
  GLShaderCache shader_cache;
  uint32_t shader_ref;
  ASSERT_FALSE(shader_cache.GetShaderReference(
      kValidVertexShaderName, &shader_ref));
  ASSERT_TRUE(shader_cache.CompileShader(
      kValidVertexShaderName,
      ShaderType::VERTEX,
      kValidVertexShaderSrc));
  ASSERT_TRUE(shader_cache.GetShaderReference(
      kValidVertexShaderName, &shader_ref));
}

TEST_F(GLShaderCacheTest, HappyPathShaderCompilationAndLink) {
  GLShaderCache shader_cache;
  ASSERT_TRUE(shader_cache.CompileShader(
      kValidVertexShaderName,
      ShaderType::VERTEX,
      kValidVertexShaderSrc));
  ASSERT_TRUE(shader_cache.CompileShader(
      kValidFragmentShaderName,
      ShaderType::FRAGMENT,
      kValidFragmentShaderSrc));
  uint32_t shader_ref;
  ASSERT_TRUE(shader_cache.GetShaderReference(
      kValidVertexShaderName, &shader_ref));
  ASSERT_TRUE(shader_cache.GetShaderReference(
      kValidFragmentShaderName, &shader_ref));
  ASSERT_TRUE(shader_cache.LinkProgram(
      kValidProgramName,
      {kValidVertexShaderName, kValidFragmentShaderName}));
  uint32_t program_ref;
  ASSERT_TRUE(shader_cache.GetProgramReference(
      kValidProgramName, &program_ref));
}

TEST_F(GLShaderCacheTest,
       ErrorOnRecompilingSameShaderSourceDifferentName) {
  GLShaderCache shader_cache;
  ASSERT_TRUE(shader_cache.CompileShader(
      kValidVertexShaderName,
      ShaderType::VERTEX,
      kValidVertexShaderSrc));
  ASSERT_FALSE(shader_cache.CompileShader(
      "different_name",
      ShaderType::VERTEX,
      kValidVertexShaderSrc));
}

TEST_F(GLShaderCacheTest, ErrorCompilingInvalidShaderSource) {
  GLShaderCache shader_cache;
  ASSERT_FALSE(shader_cache.CompileShader(
      kInvalidVertexShaderName,
      ShaderType::VERTEX,
      kInvalidVertexShaderSrc));
}

}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
