#include "renderer.h"

#include "platform/platform.cc"
#include "shader.h"
#include "ui.cc"
#include "gl/gl.cc"
#include "gl/shader.cc"
#include "math/mat_ops.h"

namespace rgg {

struct RGG {
  math::Mat4f projection;
  math::Mat4f view;
  math::Mat4f camera_transform;

  // References to shader programs.
  GLuint program_reference;

  // References to uniforms.
  GLuint matrix_uniform = -1;
  GLuint color_uniform = -1;

  // References to vertex data on GPU.
  GLuint triangle_vao_reference;
  GLuint rectangle_vao_reference;
  GLuint line_vao_reference;

  int meter_size = 50;
};

static RGG kRGG;

bool
Initialize()
{
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);

  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &rgg::kVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &rgg::kFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders({vert_shader, frag_shader}, &kRGG.program_reference)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.matrix_uniform = glGetUniformLocation(kRGG.program_reference, "matrix");
  assert(kRGG.matrix_uniform != uint32_t(-1));
  kRGG.color_uniform = glGetUniformLocation(kRGG.program_reference, "color");
  assert(kRGG.color_uniform != uint32_t(-1));

  // Create the geometry for basic shapes.

  // Triangle.
  float m = kRGG.meter_size;
  kRGG.triangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(0.0f, m / 2.f),
       math::Vec2f(m / 2.f, -m / 2.f),
       math::Vec2f(-m / 2.f, -m / 2.f)});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kRGG.rectangle_vao_reference = gl::CreateGeometryVAO(
      {math::Vec2f(-m / 2.f, m / 2.f), math::Vec2f(m / 2.f, m / 2.f),
       math::Vec2f(m / 2.f, -m / 2.f), math::Vec2f(-m / 2.f, -m / 2.f)});

  // Line is flat on the x-axis with distance m.
  kRGG.line_vao_reference =
      gl::CreateGeometryVAO({math::Vec2f(-1.f, 0.f), math::Vec2f(1.f, 0.f)});

  if (!ui::Initialize()) {
    return false;
  }

  return true;
}

void
SetProjectionMatrix(const math::Mat4f& projection)
{
  kRGG.projection  = projection;
}

void
SetViewMatrix(const math::Mat4f& view)
{
  kRGG.view = view;
}

void
SetCameraTransformMatrix(const math::Mat4f& camera_transform)
{
  kRGG.camera_transform = camera_transform;
}

void
RenderTriangle(const math::Vec3f& position, const math::Vec3f& scale,
               const math::Quatf& orientation, const math::Vec4f& color)
{
  glUseProgram(kRGG.program_reference);
  glBindVertexArray(kRGG.triangle_vao_reference);
   // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::CreateTranslationMatrix(position) *
                      math::CreateScaleMatrix(scale) *
                      math::CreateRotationMatrix(orientation);
  math::Mat4f matrix = kRGG.projection * kRGG.view * model;
  glUniform4f(kRGG.color_uniform, color.x, color.y, color.z, color.w);
  glUniformMatrix4fv(kRGG.matrix_uniform, 1, GL_FALSE, &matrix[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
}

void
RenderRectangle(const math::Vec3f& position, const math::Vec3f& scale,
                const math::Quatf& orientation, const math::Vec4f& color)
{
  glUseProgram(kRGG.program_reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the rectangle appropriately.
  math::Mat4f model = math::CreateTranslationMatrix(position) *
                      math::CreateScaleMatrix(scale) *
                      math::CreateRotationMatrix(orientation);
  math::Mat4f matrix = kRGG.projection * kRGG.view * model;
  glUniform4f(kRGG.color_uniform, color.x, color.y, color.z, color.w);
  glUniformMatrix4fv(kRGG.matrix_uniform, 1, GL_FALSE, &matrix[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
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
RenderLine(const math::Vec3f& start, const math::Vec3f& end,
           const math::Vec4f& color)
{
  glUseProgram(kRGG.program_reference);
  glBindVertexArray(kRGG.line_vao_reference);
  math::Mat4f matrix = kRGG.projection * kRGG.view *
                       CreateLineTransform(start, end);
  glUniform4f(kRGG.color_uniform, color.x, color.y, color.z, color.w);
  glUniformMatrix4fv(kRGG.matrix_uniform, 1, GL_FALSE, &matrix[0]);
  glDrawArrays(GL_LINES, 0, 2);
}

void
RenderGrid(float width, float height, const math::Vec4f& color)
{
  glUseProgram(kRGG.program_reference);
  glBindVertexArray(kRGG.line_vao_reference);
  // The bottom left and top right of the screen with regards to the camera.
  math::Vec3f top_right = kRGG.camera_transform * math::Vec3f(window::GetWindowSize());
  math::Vec3f bottom_left = kRGG.camera_transform * math::Vec3f(0.f, 0.f, 0.f);
  float right_most_grid_x =
        width - (fmod(top_right.x + width, width));
  float left_most_grid_x = -fmod(bottom_left.x + width, width);
  float top_most_grid_y =
      height - (fmod(top_right.y + height, height));
  float bottom_most_grid_y = -fmod(bottom_left.y + height, height);

  math::Vec3f top_right_transformed =
      top_right + math::Vec3f(right_most_grid_x, top_most_grid_y, 0.f);
  math::Vec3f bottom_left_transformed =
      bottom_left + math::Vec3f(left_most_grid_x, bottom_most_grid_y, 0.f);

  // Draw horizontal lines.
  for (float y = bottom_left_transformed.y; y < top_right.y;
       y += height) {
    auto start = math::Vec3f(bottom_left_transformed.x, y, 0.f);
    auto end = math::Vec3f(top_right_transformed.x, y, 0.f);
    RenderLine(start, end, color);
  }

  // Draw vertical lines.
  for (float x = bottom_left_transformed.x; x < top_right.x;
       x += width) {
    auto start = math::Vec3f(x, bottom_left_transformed.y, 0.f);
    auto end = math::Vec3f(x, top_right_transformed.y, 0.f);
    RenderLine(start, end, color);
  }
}

}
