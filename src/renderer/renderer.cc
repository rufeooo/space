#pragma once

#include "shader.h"
#include "tga_loader.cc"

#include "gl/gl.cc"
#include "math/math.cc"
#include "platform/platform.cc"

struct RenderTag {
  // TODO(abrunasso): Support custom shaders.
  GLuint vao_reference;
  GLuint vert_count;
  GLenum mode;
};

namespace rgg
{
struct GeometryProgram {
  GLuint reference = -1;
  GLuint matrix_uniform = -1;
  GLuint color_uniform = -1;
};

struct SmoothRectangleProgram {
  GLuint reference = -1;
  GLuint model_uniform = -1;
  GLuint view_projection_uniform = -1;
  GLuint color_uniform = -1;
  GLuint smoothing_radius_uniform = -1;
};

struct CircleProgram {
  GLuint reference = -1;
  GLuint model_uniform = -1;
  GLuint view_projection_uniform = -1;
  GLuint color_uniform = -1;
  GLuint inner_radius_uniform = -1;
  GLuint outer_radius_uniform = -1;
};

struct Observer {
  math::Mat4f projection;
  math::Mat4f view = math::Identity();
};

struct RGG {
  GeometryProgram geometry_program;
  SmoothRectangleProgram smooth_rectangle_program;
  CircleProgram circle_program;

  // References to vertex data on GPU.
  GLuint triangle_vao_reference;
  GLuint rectangle_vao_reference;
  GLuint line_vao_reference;

  int meter_size = 50;
};

static Observer kObserver;
static RGG kRGG;

#include "texture.cc"
#include "ui.cc"

class ModifyObserver
{
 public:
  ModifyObserver(const math::Mat4f& proj, const math::Mat4f& view)
  {
    projection_ = kObserver.projection;
    view_ = kObserver.view;
    kObserver.projection = proj;
    kObserver.view = view;
  }

  ~ModifyObserver()
  {
    kObserver.projection = projection_;
    kObserver.view = view_;
  }

 private:
  math::Mat4f projection_;
  math::Mat4f view_;
};

Observer*
GetObserver()
{
  return &kObserver;
}

bool
SetupGeometryProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kVertexShader, &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kFragmentShader, &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.geometry_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  if (!gl::CompileShader(GL_VERTEX_SHADER, &kSmoothRectangleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kSmoothRectangleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.smooth_rectangle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program.matrix_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "matrix");
  assert(kRGG.geometry_program.matrix_uniform != uint32_t(-1));
  kRGG.geometry_program.color_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "color");
  assert(kRGG.geometry_program.color_uniform != uint32_t(-1));

  kRGG.smooth_rectangle_program.model_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "model");
  assert(kRGG.smooth_rectangle_program.model_uniform != uint32_t(-1));
  kRGG.smooth_rectangle_program.view_projection_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "view_projection");
  assert(kRGG.smooth_rectangle_program.view_projection_uniform != uint32_t(-1));

  kRGG.smooth_rectangle_program.color_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "color");
  assert(kRGG.smooth_rectangle_program.color_uniform != uint32_t(-1));
  kRGG.smooth_rectangle_program.smoothing_radius_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "smoothing_radius");
  assert(kRGG.smooth_rectangle_program.smoothing_radius_uniform !=
         uint32_t(-1));

  return true;
}

bool
SetupCircleProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kCircleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kCircleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.circle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.circle_program.model_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "model");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));
  kRGG.circle_program.view_projection_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "view_projection");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));

  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  kRGG.circle_program.inner_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "inner_radius");
  assert(kRGG.circle_program.inner_radius_uniform != uint32_t(-1));
  kRGG.circle_program.outer_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "outer_radius");
  assert(kRGG.circle_program.outer_radius_uniform != uint32_t(-1));
  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  assert(kRGG.circle_program.color_uniform != uint32_t(-1));
  return true;
}

bool
Initialize()
{
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("Renderer: %s Version: %s\n", renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);

  // Compile and link shaders.
  if (!SetupGeometryProgram()) return false;
  if (!SetupCircleProgram()) return false;

  // Create the geometry for basic shapes.
  float m = kRGG.meter_size;
  GLfloat tri[9] = {0.0f, m / 2.f,  0.f,      m / 2.f, -m / 2.f,
                    0.f,  -m / 2.f, -m / 2.f, 0.f};
  kRGG.triangle_vao_reference = gl::CreateGeometryVAO(9, tri);

  // Rectangle. Notice it's a square. Scale to make rectangly.
  // clang-format off
  static GLfloat square[18] = {
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f,

      -m / 2.f, -m / 2.f, 0.f,
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f
  };
  // clang-format on
  kRGG.rectangle_vao_reference = gl::CreateGeometryVAO(18, square);

  // Line is flat on the x-axis with distance m.
  GLfloat line[6] = {-1.f, 0.f, 0.f, 1.f, 0.f, 0.f};
  kRGG.line_vao_reference = gl::CreateGeometryVAO(6, line);

  if (!SetupTexture()) {
    printf("Failed to setup Texture.\n");
    return false;
  }

  if (!SetupUI()) {
    printf("Failed to setup UI.\n");
    return false;
  }

  return true;
}

RenderTag
CreateRenderable(int vert_count, GLfloat* verts, GLenum mode)
{
  RenderTag tag = {};
  tag.vao_reference = gl::CreateGeometryVAO(vert_count * 3, verts);
  tag.vert_count = vert_count;
  tag.mode = mode;
  return tag;
}

void
RenderTag(const RenderTag& tag, const v3f& position, const v3f& scale,
          const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(tag.vao_reference);
  // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(tag.mode, 0, tag.vert_count);
}

void
RenderTriangle(const v3f& position, const v3f& scale,
               const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.triangle_vao_reference);
  // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
}

void
RenderRectangle(const v3f& position, const v3f& scale,
                const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the rectangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderRectangle(const math::Rect& rect, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderLineRectangle(const math::Rect& rect, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void
RenderSmoothRectangle(const math::Rect& rect, float smoothing_radius,
                      const v4f& color)
{
  glUseProgram(kRGG.smooth_rectangle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f view_projection = kObserver.projection * kObserver.view;
  glUniform1f(kRGG.smooth_rectangle_program.smoothing_radius_uniform,
              rect.width - smoothing_radius);
  glUniform4f(kRGG.smooth_rectangle_program.color_uniform, color.x, color.y,
              color.z, color.w);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.view_projection_uniform, 1,
                     GL_FALSE, &view_projection.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderCircle(const v3f& position, float inner_radius, float outer_radius,
             const v4f& color)
{
  glUseProgram(kRGG.circle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  // Translate and rotate the circle appropriately.
  math::Mat4f model =
      math::Model(position, v3f(outer_radius * 2.f, outer_radius * 2.f, 0.0f));
  math::Mat4f view_pojection = kObserver.projection * kObserver.view;
  glUniform1f(kRGG.circle_program.inner_radius_uniform, inner_radius);
  glUniform1f(kRGG.circle_program.outer_radius_uniform, outer_radius);
  glUniform4f(kRGG.circle_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.circle_program.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniformMatrix4fv(kRGG.circle_program.view_projection_uniform, 1, GL_FALSE,
                     &view_pojection.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderCircle(const v3f& position, float radius, const v4f& color)
{
  RenderCircle(position, 0.0f, radius, color);
}

math::Mat4f
CreateLineTransform(const v3f& start, const v3f& end)
{
  // Line is the length of a meter on the horizontal axis.
  //
  // It must be translated / rotated / scaled to be properly moved
  // to meet the start / end nature of the line component.

  // Position is the midpoint of the start and end.
  v3f translation = (start + end) / 2.f;
  // Angle between the two points in 2d.
  v3f diff = end - start;
  float angle = atan2(diff.y, diff.x) * (180.f / PI);
  float distance = math::Length(diff);
  return math::Model(translation, v3f(distance / 2.f, distance / 2.f, 1.f),
                     math::Quatf(angle, v3f(0.f, 0.f, -1.f)));
}

void
RenderLine(const v3f& start, const v3f& end, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);
  math::Mat4f matrix =
      kObserver.projection * kObserver.view * CreateLineTransform(start, end);
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINES, 0, 2);
}

void
RenderGrid(v2f grid, math::Rect bounds, uint64_t color_count, v4f* color)
{
  // Prepare Geometry and color
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);

  // Draw horizontal lines.
  const v2f top_right(bounds.x + bounds.width, bounds.y + bounds.height);
  int i = 0;
  for (float y = bounds.y; y <= top_right.y; y += grid.y) {
    auto start = v3f(bounds.x, y, 0.f);
    auto end = v3f(top_right.x, y, 0.f);
    math::Mat4f matrix =
        kObserver.projection * kObserver.view * CreateLineTransform(start, end);
    glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                       &matrix.data_[0]);

    glUniform4f(kRGG.geometry_program.color_uniform, color[i].x, color[i].y,
                color[i].z, color[i].w);
    glDrawArrays(GL_LINES, 0, 2);

    i += 1;
    i = (i != color_count) * i;
  }

  // Draw vertical lines.
  i = 0;
  for (float x = bounds.x; x <= top_right.x; x += grid.x) {
    auto start = v3f(x, bounds.y, 0.f);
    auto end = v3f(x, top_right.y, 0.f);
    math::Mat4f matrix =
        kObserver.projection * kObserver.view * CreateLineTransform(start, end);
    glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                       &matrix.data_[0]);
    glUniform4f(kRGG.geometry_program.color_uniform, color[i].x, color[i].y,
                color[i].z, color[i].w);
    glDrawArrays(GL_LINES, 0, 2);

    i += 1;
    i = (i != color_count) * i;
  }
}

}  // namespace rgg
