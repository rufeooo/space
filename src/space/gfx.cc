#include "gfx.h"

#include "camera.cc"
#include "entity.cc"
#include "renderer/renderer.cc"

namespace gfx
{
CreateProjectionFunctor* _custom_projection = nullptr;

constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;

constexpr int kMaxTriangleCount = 10;
constexpr int kMaxRectangleCount = 10;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct Triangle {
  math::Vec3f position;
  math::Vec3f scale;
  math::Quatf orientation;
  math::Vec4f color;
};

struct Rectangle {
  math::Vec3f position;
  math::Vec3f scale;
  math::Quatf orientation;
  math::Vec4f color;
};

struct Gfx {
  // Allow 32 on screen text messages.
  Text text[kMaxTextCount];
  int text_count;

  Triangle triangle[kMaxTriangleCount];
  int triangle_count;

  Rectangle rectangle[kMaxRectangleCount];
  int rectangle_count;
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
  int window_result = window::Create("Space", 800, 800);
  printf("Window create result: %i\n", window_result);
  return rgg::Initialize();
}

void
RenderTriangles()
{
  for (int i = 0; i < kGfx.triangle_count; ++i) {
    Triangle* tri = &kGfx.triangle[i];
    rgg::RenderTriangle(tri->position, tri->scale, tri->orientation,
                        tri->color);
  }
  kGfx.triangle_count = 0;
}

void
RenderRectangles()
{
  for (int i = 0; i < kGfx.rectangle_count; ++i) {
    Rectangle* rect = &kGfx.rectangle[i];
    rgg::RenderRectangle(rect->position, rect->scale, rect->orientation,
                         rect->color);
  }
  kGfx.rectangle_count = 0;
}

void
RenderLines()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (!EntityExists(i)) break;
    if (!COMPONENT_EXISTS(i, line)) continue;

    Entity* ent = &game_entity[i];
    rgg::RenderLine(ent->line.start, ent->line.end, ent->line.color);
  }
}

void
RenderGrids()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (!EntityExists(i)) break;
    if (!COMPONENT_EXISTS(i, grid)) continue;

    Entity* ent = &game_entity[i];
    rgg::RenderGrid(ent->grid.width, ent->grid.height, ent->grid.color);
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
  RenderRectangles();
  RenderLines();
  RenderGrids();
  // Undo wireframe drawing.
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Draw all text.
  glClear(GL_DEPTH_BUFFER_BIT);
  for (int i = 0; i < kGfx.text_count; ++i) {
    Text& text = kGfx.text[i];
    ui::Text(text.msg, text.screen_x, text.screen_y, math::Vec4f());
  }

  // Reset draw pointers.
  kGfx.text_count = 0;  
  
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
PushTriangle(const math::Vec3f& position, const math::Vec3f& scale,
             const math::Quatf& orientation, const math::Vec4f& color)
{
  assert(kGfx.triangle_count + 1 < kMaxTriangleCount);
  if (kGfx.triangle_count + 1 >= kMaxTriangleCount) return;
  SetPrimitive(position, scale, orientation, color,
               &kGfx.triangle[kGfx.triangle_count++]);
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
