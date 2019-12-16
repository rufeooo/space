#include "gfx.h"

#include <iostream>

#include "ecs.h"
#include "math/vec.h"
#include "math/mat_ops.h"
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

  int current_mouse_state;
  int previous_mouse_state;
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
      math::Vec2f( 0.0f  , 0.125f ),
      math::Vec2f( 0.125f, -0.125f),
      math::Vec2f(-0.125f, -0.125f)});

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
  int state = glfwGetMouseButton(kOpenGL.glfw, GLFW_MOUSE_BUTTON_LEFT);
  kOpenGL.previous_mouse_state = kOpenGL.current_mouse_state;
  kOpenGL.current_mouse_state = state;
}

bool Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw all tirangles
  glUseProgram(kOpenGL.program_reference);
  glBindVertexArray(kOpenGL.triangle_vao_reference);
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity,
          TransformComponent& transform, TriangleComponent& triangle) {
    // Translate and rotate the triangle appropriately.
    math::Mat4f matrix =
        math::CreateTranslationMatrix(transform.position);
        math::CreateRotationMatrix(transform.orientation);
    glUniformMatrix4fv(kOpenGL.matrix_uniform, 1, GL_FALSE, &matrix[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
  });
  glfwSwapBuffers(kOpenGL.glfw);
  return !glfwWindowShouldClose(kOpenGL.glfw);
}

math::Vec2f GetCursorPosition() {
  double xpos, ypos;
  glfwGetCursorPos(kOpenGL.glfw, &xpos, &ypos);
  return math::Vec2f((float)xpos, (float)ypos);
}


math::Vec2f GetCursorPositionInGLSpace() {
  // Get cursor. Top left (0,0) bottom right (window_width, window_height)
  math::Vec2f cursor = GetCursorPosition();
  // Convert to opengl screen space horizontal [-1, 1] vertical [-1, 1]
  //
  // 1. Convert cursor to top left (0,0) -> bottom right (2,2)
  cursor /= gfx::GetWindowDims() / 2.f;
  // 2. Subtract 1.f from both components to get (-1,-1) -> (1,1)
  cursor -= 1.f;
  // 3. Invert the y-axis since GL has (0,0) in middle and (-1,1) in top left.
  cursor.y = -cursor.y;
  return cursor;
}


math::Vec2f GetWindowDims() {
  int width, height;
  glfwGetWindowSize(kOpenGL.glfw, &width, &height);
  return math::Vec2f((float)width, (float)height);
}


bool LeftMouseClicked() {
  return kOpenGL.current_mouse_state == GLFW_RELEASE &&
         kOpenGL.previous_mouse_state == GLFW_PRESS;
}

}
