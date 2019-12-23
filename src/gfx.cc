#include "gfx.h"

#include <iostream>

#include "camera.h"
#include "ecs.h"
#include "math/mat_ops.h"
#include "math/vec.h"
#include "gl/shader_cache.h"
#include "gl/utils.h"

namespace gfx
{
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
  gl::ShaderCache shader_cache;

  // References to shader programs.
  uint32_t program_reference;

  // References to uniforms.
  uint32_t matrix_uniform;

  // References to vertex data on GPU.
  uint32_t triangle_vao_reference;
  uint32_t rectangle_vao_reference;
  uint32_t line_vao_reference;

  // Number of pixels that correspond with a meter.
  int meter_size = 50;
};

static Gfx kGfx;

bool
Initialize()
{
  gl::InitGLAndCreateWindow(800, 800, "Space");

  if (!kGfx.shader_cache.CompileShader(
          kVertexShaderName, gl::ShaderType::VERTEX, kVertexShader)) {
    std::cout << "Unable to compile " << kVertexShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(kFragmentShaderName,
                                       gl::ShaderType::FRAGMENT,
                                       kFragmentShader)) {
    std::cout << "Unable to compile " << kFragmentShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.LinkProgram(
          kProgramName, {kVertexShaderName, kFragmentShaderName})) {
    std::cout << "Unable to link: " << kProgramName
              << " info: " << kGfx.shader_cache.GetProgramInfo(kProgramName)
              << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.GetProgramReference(kProgramName,
                                             &kGfx.program_reference)) {
    return false;
  }

  kGfx.matrix_uniform = glGetUniformLocation(kGfx.program_reference, "matrix");

  // Create the geometry for basic shapes.

  // Triangle.
  float m = kGfx.meter_size;
  kGfx.triangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(0.0f, m / 2.f),
       math::Vec2f(m / 2.f, -m / 2.f),
       math::Vec2f(-m / 2.f, -m / 2.f)});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kGfx.rectangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(-m / 2.f, m / 2.f), math::Vec2f(m / 2.f, m / 2.f),
       math::Vec2f(m / 2.f, -m / 2.f), math::Vec2f(-m / 2.f, -m / 2.f)});

  // Line is flat on the x-axis with distance m.
  kGfx.line_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(-m / 2.f, 0.f), math::Vec2f(m / 2.f, 0.f)});

  return true;
}

bool
Render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  math::Vec2f dims = window::GetWindowSize();
  // TODO: Take into consideration camera.
  math::Mat4f ortho = math::CreateOrthographicMatrix<float>(
      dims.x, 0.f, dims.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  math::Mat4f view = camera::view_matrix();
  math::Mat4f ortho_view = ortho * view;

  // Draw all triangles
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.triangle_vao_reference);
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          TriangleComponent&) {
        // Translate and rotate the triangle appropriately.
        math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                            math::CreateScaleMatrix(transform.scale) *
                            math::CreateRotationMatrix(transform.orientation);
        math::Mat4f matrix = ortho_view * model;
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINE_LOOP, 0, 3);
      });

  // Draw all rectangles.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.rectangle_vao_reference);
  kECS.Enumerate<TransformComponent, RectangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          RectangleComponent&) {
        // Translate and rotate the rectangle appropriately.
        math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                            math::CreateScaleMatrix(transform.scale) *
                            math::CreateRotationMatrix(transform.orientation);
        math::Mat4f matrix = ortho_view * model;
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
      });

  // Draw all lines.
  // TODO: This should eventually work with 3d too.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.line_vao_reference);
  kECS.Enumerate<LineComponent>(
      [&](ecs::Entity entity, LineComponent& line) {
        // Line is the length of a meter on the horizontal axis.
        //
        // It must be translated / rotated / scaled to be properly moved
        // to meet the start / end nature of the line component.
        
        // Position is the midpoint of the start and end.
        math::Vec3f midpoint = line.start + ((line.end - line.start) / 2.f);
        // The scaling  factor is the distance between the start and end.
        // Divide by kGfx.meter_size since the line is defined in terms of the
        // meter.
        float distance = math::Length(line.end - line.start) / kGfx.meter_size;
        // Angle between the two points in 2d.
        float angle = std::atan2(line.end.y - line.start.y,
                                 line.end.x - line.start.x) * 180.f / PI;

        // Translate and rotate the rectangle appropriately.
        math::Mat4f model = math::CreateTranslationMatrix(midpoint) *
                            math::CreateScaleMatrix(
                                math::Vec3f(distance, distance, 1.f)) *
                            math::CreateRotationMatrix(
                                math::Quatf(angle, math::Vec3f(0.f, 0.f, -1.f)));

        math::Mat4f matrix = ortho_view * model;
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINES, 0, 2);
      });


  window::SwapBuffers();
  return true;
}

}  // namespace gfx
