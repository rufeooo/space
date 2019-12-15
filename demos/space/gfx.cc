#include "gfx.h"

#include <iostream>

#include "math/vec.h"
#include "renderer/gl_utils.h"
#include "renderer/gl_shader_cache.h"

namespace gfx {

constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  void main() {
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

constexpr const char* kVertexShaderName = "vert";

constexpr const char* kFragmentShader = R"(
  #version 410
	out vec4 frag_color;
  void main() {
   frag_color = vec4(1.0, 1.0, 1.0, 1.0);
  }
)";

constexpr const char* kFragmentShaderName = "frag";

constexpr const char* kProgramName = "prog";

struct OpenGL {
  GLFWwindow* glfw = nullptr;
  renderer::GLShaderCache shader_cache;

  // References to shader programs.
  uint32_t program_reference;

  // References to uniforms.
  uint32_t matrix_uniform;

  // References to vertex data on GPU.
  uint32_t triangle_vao_reference;
  uint32_t rectangle_vao_reference;
};

static OpenGL kOpenGL;

bool Initialize() {
  kOpenGL.glfw = renderer::InitGLAndCreateWindow(800, 800, "Space");
  if (!kOpenGL.glfw) {
    std::cout << "Unable to start GL and create window."
              << std::endl;
    return false;
  }

  if (!kOpenGL.shader_cache.CompileShader(
        kVertexShaderName, renderer::ShaderType::VERTEX, kVertexShader)) {
    std::cout << "Unable to compile " << kVertexShaderName << std::endl;
    return false;
  }

  if (!kOpenGL.shader_cache.CompileShader(
      kFragmentShaderName, renderer::ShaderType::FRAGMENT, kFragmentShader)) {
    std::cout << "Unable to compile " << kFragmentShaderName << std::endl;
    return false;
  }

  if (!kOpenGL.shader_cache.LinkProgram(
        kProgramName, { kVertexShaderName, kFragmentShaderName })) {
    std::cout << "Unable to link: " << kProgramName << " info: "
              << kOpenGL.shader_cache.GetProgramInfo(kProgramName)
              << std::endl;
    return false;
  }

  if (!kOpenGL.shader_cache.GetProgramReference(
        kProgramName, &kOpenGL.program_reference)) {
    return false;
  }

  kOpenGL.matrix_uniform =
      glGetUniformLocation(kOpenGL.program_reference, "matrix");

  // Create the geometry for basic shapes.

  // Triangle.
  kOpenGL.triangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f( 0.0f , 0.25f),
      math::Vec2f( 0.25f, 0.0f ),
      math::Vec2f(-0.25f, 0.0f )});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kOpenGL.rectangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f(-0.25f,  0.25f),
      math::Vec2f( 0.25f,  0.25f),
      math::Vec2f( 0.25f, -0.25f),
      math::Vec2f(-0.25f, -0.25f)});

  return true;
}

void PollEvents() {
  glfwPollEvents();
}

bool Render() {
  glfwSwapBuffers(kOpenGL.glfw);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return !glfwWindowShouldClose(kOpenGL.glfw);
}

}
