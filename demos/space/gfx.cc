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

struct Gfx {
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

static Gfx kGfx;

bool Initialize() {
  kGfx.glfw = renderer::InitGLAndCreateWindow(800, 800, "Space");
  if (!kGfx.glfw) {
    std::cout << "Unable to start GL and create window."
              << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(
        kVertexShaderName, renderer::ShaderType::VERTEX, kVertexShader)) {
    std::cout << "Unable to compile " << kVertexShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(
      kFragmentShaderName, renderer::ShaderType::FRAGMENT, kFragmentShader)) {
    std::cout << "Unable to compile " << kFragmentShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.LinkProgram(
        kProgramName, { kVertexShaderName, kFragmentShaderName })) {
    std::cout << "Unable to link: " << kProgramName << " info: "
              << kGfx.shader_cache.GetProgramInfo(kProgramName)
              << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.GetProgramReference(
        kProgramName, &kGfx.program_reference)) {
    return false;
  }

  kGfx.matrix_uniform =
      glGetUniformLocation(kGfx.program_reference, "matrix");

  // Create the geometry for basic shapes.

  // Triangle.
  kGfx.triangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f( 0.0f  , 0.125f ),
      math::Vec2f( 0.125f, -0.125f),
      math::Vec2f(-0.125f, -0.125f)});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kGfx.rectangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f(-0.0625f,  0.0625f),
      math::Vec2f( 0.0625f,  0.0625f),
      math::Vec2f( 0.0625f, -0.0625f),
      math::Vec2f(-0.0625f, -0.0625f)});

  return true;
}

void PollEvents() {
  glfwPollEvents();
  int state = glfwGetMouseButton(kGfx.glfw, GLFW_MOUSE_BUTTON_LEFT);
  kGfx.previous_mouse_state = kGfx.current_mouse_state;
  kGfx.current_mouse_state = state;
}

bool Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw all triangles
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.triangle_vao_reference);
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity,
          TransformComponent& transform, TriangleComponent&) {
    // Translate and rotate the triangle appropriately.
    math::Mat4f matrix = math::CreateTranslationMatrix(transform.position) *
                         math::CreateRotationMatrix(transform.orientation);
    glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
  });

  // Draw all rectangles.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.rectangle_vao_reference);
  kECS.Enumerate<TransformComponent, RectangleComponent>(
      [&](ecs::Entity entity,
          TransformComponent& transform, RectangleComponent&) {
    // Translate and rotate the rectangle appropriately.
    math::Mat4f matrix = math::CreateTranslationMatrix(transform.position) *
                         math::CreateRotationMatrix(transform.orientation);
    glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  });

  glfwSwapBuffers(kGfx.glfw);
  return !glfwWindowShouldClose(kGfx.glfw);
}

math::Vec2f GetCursorPosition() {
  double xpos, ypos;
  glfwGetCursorPos(kGfx.glfw, &xpos, &ypos);
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
  glfwGetWindowSize(kGfx.glfw, &width, &height);
  return math::Vec2f((float)width, (float)height);
}


bool LeftMouseClicked() {
  return kGfx.current_mouse_state == GLFW_RELEASE &&
         kGfx.previous_mouse_state == GLFW_PRESS;
}

}
