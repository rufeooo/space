#include "gfx.h"

#include <cmath>
#include <iostream>

#include "camera.h"
#include "ecs.h"
#include "gl/renderer.cc"
#include "math/math.cc"

namespace gfx
{
constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  uniform vec4 color;
  out vec4 color_out;
  void main() {
    color_out = color;
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

constexpr const char* kVertexShaderName = "vert";

constexpr const char* kFragmentShader = R"(
  #version 410
  in vec4 color_out;
	out vec4 frag_color;
  void main() {
   frag_color = color_out;
  }
)";

constexpr const char* kFragmentShaderName = "frag";

constexpr const char* kProgramName = "prog";

struct Gfx {
  gl::ShaderCache shader_cache;

  // References to shader programs.
  uint32_t program_reference;

  // References to uniforms.
  uint32_t matrix_uniform = -1;
  uint32_t color_uniform = -1;

  // References to vertex data on GPU.
  uint32_t triangle_vao_reference;
  uint32_t rectangle_vao_reference;
  uint32_t line_vao_reference;

  // Number of pixels that correspond with a meter.
  int meter_size = 50;
};

static Gfx kGfx;

CreateProjectionFunctor* _custom_projection = nullptr;

void
SetProjection(CreateProjectionFunctor* projection)
{
  _custom_projection = projection;
}

bool
Initialize()
{
  gl::InitGLAndCreateWindow(800, 800, "Space");

  if (!kGfx.shader_cache.CompileShader(kVertexShaderName,
                                       gl::ShaderType::VERTEX, kVertexShader)) {
    std::cout << "Unable to compile " << kVertexShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(
          kFragmentShaderName, gl::ShaderType::FRAGMENT, kFragmentShader)) {
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
  assert(kGfx.matrix_uniform != uint32_t(-1));
  kGfx.color_uniform = glGetUniformLocation(kGfx.program_reference, "color");
  assert(kGfx.color_uniform != uint32_t(-1));

  // Create the geometry for basic shapes.

  // Triangle.
  float m = kGfx.meter_size;
  kGfx.triangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(0.0f, m / 2.f), math::Vec2f(m / 2.f, -m / 2.f),
       math::Vec2f(-m / 2.f, -m / 2.f)});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kGfx.rectangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(-m / 2.f, m / 2.f), math::Vec2f(m / 2.f, m / 2.f),
       math::Vec2f(m / 2.f, -m / 2.f), math::Vec2f(-m / 2.f, -m / 2.f)});

  // Line is flat on the x-axis with distance m.
  kGfx.line_vao_reference =
      gl::CreateGeometryVAO({math::Vec2f(-1.f, 0.f), math::Vec2f(1.f, 0.f)});

  return true;
}

void
RenderTriangles(const math::Mat4f& ortho_view)
{
  // Draw all triangles
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.triangle_vao_reference);
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          TriangleComponent& tri) {
        // Translate and rotate the triangle appropriately.
        math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                            math::CreateScaleMatrix(transform.scale) *
                            math::CreateRotationMatrix(transform.orientation);
        math::Mat4f matrix = ortho_view * model;
        glUniform4f(kGfx.color_uniform, tri.color.x, tri.color.y,
                    tri.color.z, tri.color.w);
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINE_LOOP, 0, 3);
      });
}

void
RenderRectangles(const math::Mat4f& ortho_view)
{
  // Draw all rectangles.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.rectangle_vao_reference);
  kECS.Enumerate<TransformComponent, RectangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          RectangleComponent& rect) {
        // Translate and rotate the rectangle appropriately.
        math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                            math::CreateScaleMatrix(transform.scale) *
                            math::CreateRotationMatrix(transform.orientation);
        math::Mat4f matrix = ortho_view * model;
        glUniform4f(kGfx.color_uniform, rect.color.x, rect.color.y,
                    rect.color.z, rect.color.w);
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
      });
}

math::Mat4f
CreateLineTransform(const math::Vec3f& start, const math::Vec3f& end)
{
  // Line is the length of a meter on the horizontal axis.
  //
  // It must be translated / rotated / scaled to be properly moved
  // to meet the start / end nature of the line component.

  // Position is the midpoint of the start and end.
  math::Vec3f translation = (start + end) / 2.f;
  // Angle between the two points in 2d.
  math::Vec3f diff = end - start;
  float angle = std::atan2(diff.y, diff.x) * (180.f / PI);
  float distance = math::Length(diff);
  return math::CreateTranslationMatrix(translation) *
         math::CreateScaleMatrix(
             math::Vec3f(distance / 2.f, distance / 2.f, 1.f)) *
         math::CreateRotationMatrix(
             math::Quatf(angle, math::Vec3f(0.f, 0.f, -1.f)));
}

void
RenderLines(const math::Mat4f& ortho_view)
{
  // Draw all lines.
  // TODO: This should eventually work with 3d too.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.line_vao_reference);
  kECS.Enumerate<LineComponent>(
      [&](ecs::Entity entity, LineComponent& line) {
        math::Mat4f matrix = ortho_view * CreateLineTransform(line.start, line.end);
        glUniform4f(kGfx.color_uniform, line.color.x, line.color.y,
                    line.color.z, line.color.w);
        glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
        glDrawArrays(GL_LINES, 0, 2);
      });
}

void
RenderGrids(const math::Mat4f& ortho_view, const math::Vec2f& dims)
{
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.line_vao_reference);

  math::Mat4f camera_transform = camera::transform_matrix();
  // The bottom left and top right of the screen with regards to the camera.
  math::Vec3f top_right = camera_transform * math::Vec3f(dims);
  math::Vec3f bottom_left = camera_transform * math::Vec3f(0.f, 0.f, 0.f);

  kECS.Enumerate<GridComponent>(
      [&](ecs::Entity entity, GridComponent& grid) {
        // This was painful to calculate.... :(
        //
        // These is calculating the extents of the grid to render. It should start and
        // end one grid position past the extents of the screen snapped properly to
        // a grid coordinate.
        float right_most_grid_x =
            grid.width - (fmod(top_right.x + grid.width, grid.width));
        float left_most_grid_x = -fmod(bottom_left.x + grid.width, grid.width);
        float top_most_grid_y =
            grid.height - (fmod(top_right.y + grid.height, grid.height)); 
        float bottom_most_grid_y = -fmod(bottom_left.y + grid.height, grid.height);

        math::Vec3f top_right_transformed =
          top_right + math::Vec3f(right_most_grid_x, top_most_grid_y, 0.f);
        math::Vec3f bottom_left_transformed =
          bottom_left + math::Vec3f(left_most_grid_x, bottom_most_grid_y, 0.f);
    
        glUniform4f(kGfx.color_uniform, grid.color.x, grid.color.y,
                    grid.color.z, grid.color.w);

        // Draw horizontal lines.
        for (float y = bottom_left_transformed.y; y < top_right.y; y += grid.height) {
          LineComponent line;
          line.start = math::Vec3f(bottom_left_transformed.x, y, 0.f);
          line.end = math::Vec3f(top_right_transformed.x, y, 0.f);
          math::Mat4f matrix = ortho_view * CreateLineTransform(line.start, line.end);
          glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
          glDrawArrays(GL_LINES, 0, 2);
        }

        // Draw vertical lines.
        for (float x = bottom_left_transformed.x; x < top_right.x; x += grid.width) {
          LineComponent line;
          line.start = math::Vec3f(x, bottom_left_transformed.y, 0.f);
          line.end = math::Vec3f(x, top_right_transformed.y, 0.f);
          math::Mat4f matrix = ortho_view * CreateLineTransform(line.start, line.end);
          glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
          glDrawArrays(GL_LINES, 0, 2);
        }
      });
}

bool
Render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  math::Vec2f dims = window::GetWindowSize();
  // TODO: Take into consideration camera.
  math::Mat4f projection;
  if (!_custom_projection) {
    projection = math::CreateOrthographicMatrix<float>(
        dims.x, 0.f, dims.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  } else {
    projection = _custom_projection();
  }
  math::Mat4f view = camera::view_matrix();
  math::Mat4f ortho_view = projection * view;

  // For now draw all primitives as wireframe.
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  RenderTriangles(ortho_view);
  RenderRectangles(ortho_view);
  RenderLines(ortho_view);
  RenderGrids(ortho_view, dims);
  // Undo wireframe drawing.
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  window::SwapBuffers();
  return true;
}

}  // namespace gfx
