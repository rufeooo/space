#include "gfx.h"

#include "camera.cc"
#include "entity.cc"
#include "renderer/renderer.cc"

namespace gfx
{
CreateProjectionFunctor* _custom_projection = nullptr;

constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;

constexpr int kMaxTriangleCount = 32;
// We are going to use a lot of these for tiles.
constexpr int kMaxRectangleCount = 256; 
constexpr int kMaxCircleCount = 32;
constexpr int kMaxLineCount = 32;
constexpr int kMaxGridCount = 8;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct Primitive {
  math::Vec3f position;
  math::Vec3f scale;
  math::Quatf orientation;
  math::Vec4f color;
};

struct Line {
  math::Vec3f start;
  math::Vec3f end;
  math::Vec4f color;
};

struct Grid {
  float width;
  float height;
  math::Vec4f color;
};

struct Gfx {
  // Allow 32 on screen text messages.
  Text text[kMaxTextCount];
  int text_count;

  Primitive triangle[kMaxTriangleCount];
  int triangle_count;

  Primitive rectangle[kMaxRectangleCount];
  int rectangle_count;

  Primitive circle[kMaxCircleCount];
  int circle_count;

  Line line[kMaxLineCount];
  int line_count;

  Grid grid[kMaxGridCount];
  int grid_count;
};

static Gfx kGfx;

void
SetProjection(CreateProjectionFunctor* projection)
{
  _custom_projection = projection;
}

bool
Initialize()
{
  int window_result = window::Create("Space", 1280, 720);
  printf("Window create result: %i\n", window_result);
  return rgg::Initialize();
}

void
RenderTriangles()
{
  for (int i = 0; i < kGfx.triangle_count; ++i) {
    Primitive* tri = &kGfx.triangle[i];
    rgg::RenderTriangle(tri->position, tri->scale, tri->orientation,
                        tri->color);
  }
}

void
RenderRectangles()
{
  for (int i = 0; i < kGfx.rectangle_count; ++i) {
    Primitive* rect = &kGfx.rectangle[i];
    rgg::RenderRectangle(rect->position, rect->scale, rect->orientation,
                         rect->color);
  }
}

void
RenderCircles()
{
  for (int i = 0; i < kGfx.circle_count; ++i) {
    Primitive* circ = &kGfx.circle[i];
    rgg::RenderCircle(circ->position, circ->scale, circ->orientation,
                      circ->color);
  }
}

void
RenderLines()
{
  for (int i = 0; i < kGfx.line_count; ++i) {
    Line* line = &kGfx.line[i];
    rgg::RenderLine(line->start, line->end, line->color);
  }
}

void
RenderGrids()
{
  for (int i = 0; i < kGfx.grid_count; ++i) {
    Grid* grid = &kGfx.grid[i];
    rgg::RenderGrid(grid->width, grid->height, grid->color);
  }
}

void
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
  rgg::SetProjectionMatrix(projection);
  rgg::SetViewMatrix(camera::view_matrix());
  rgg::SetCameraTransformMatrix(camera::transform_matrix());

  // For now draw all primitives as wireframe.
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  RenderTriangles();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  RenderRectangles();
  RenderCircles();
  RenderLines();
  RenderGrids();

  // Draw all text.
  glClear(GL_DEPTH_BUFFER_BIT);
  for (int i = 0; i < kGfx.text_count; ++i) {
    Text& text = kGfx.text[i];
    rgg::RenderText(text.msg, text.screen_x, text.screen_y, math::Vec4f());
  }
  
  window::SwapBuffers();
}

template <typename T>
void
SetPrimitive(const math::Vec3f& position, const math::Vec3f& scale,
             const math::Quatf& orientation, const math::Vec4f& color,
             T* primitive)
{
  primitive->position = position;
  primitive->scale = scale;
  primitive->orientation = orientation;
  primitive->color = color;
}


void
ResetRenderData()
{
  kGfx.rectangle_count = 0;
  kGfx.line_count = 0;
  kGfx.grid_count = 0;
  kGfx.triangle_count = 0;
  kGfx.circle_count = 0;
  // Reset draw pointers.
  kGfx.text_count = 0;
}

void
PushTriangle(const math::Vec3f& position, const math::Vec3f& scale,
             const math::Quatf& orientation, const math::Vec4f& color)
{
  assert(kGfx.triangle_count + 1 < kMaxTriangleCount);
  if (kGfx.triangle_count + 1 >= kMaxTriangleCount) return;
  SetPrimitive(position, scale, orientation, color,
               &kGfx.triangle[kGfx.triangle_count++]);
}

void
PushCircle(const math::Vec3f& position, const math::Vec3f& scale,
           const math::Quatf& orientation, const math::Vec4f& color)
{
  assert(kGfx.circle_count + 1 < kMaxCircleCount);
  if (kGfx.circle_count + 1 >= kMaxCircleCount) return;
  SetPrimitive(position, scale, orientation, color,
               &kGfx.circle[kGfx.circle_count++]);
}


void
PushRectangle(const math::Vec3f& position, const math::Vec3f& scale,
              const math::Quatf& orientation, const math::Vec4f& color)
{
  assert(kGfx.rectangle_count + 1 < kMaxRectangleCount);
  if (kGfx.rectangle_count + 1 >= kMaxRectangleCount) return;
  SetPrimitive(position, scale, orientation, color,
               &kGfx.rectangle[kGfx.rectangle_count++]);
}

void
PushLine(const math::Vec3f& start, const math::Vec3f& end,
         const math::Vec4f& color)
{
  assert(kGfx.line_count + 1 < kMaxLineCount);
  if (kGfx.line_count + 1 >= kMaxLineCount) return;
  Line* line = &kGfx.line[kGfx.line_count++];
  line->start = start;
  line->end = end;
  line->color = color;
}

void
PushGrid(float width, float height, const math::Vec4f& color)
{

  assert(kGfx.grid_count + 1 < kMaxGridCount);
  if (kGfx.grid_count + 1 >= kMaxGridCount) return;
  Grid* grid = &kGfx.grid[kGfx.grid_count++];
  grid->width = width;
  grid->height = height;
  grid->color = color;
}


void
PushText(const char* msg, float screen_x, float screen_y)
{
  assert(kGfx.text_count + 1 < kMaxTextCount);
  if (kGfx.text_count + 1 >= kMaxTextCount) return;
  int len = strlen(msg);
  if (len > kMaxTextSize) return;
  Text& text = kGfx.text[kGfx.text_count++];
  strcpy(text.msg, msg);
  text.screen_x = screen_x;
  text.screen_y = screen_y;
}

}  // namespace gfx
