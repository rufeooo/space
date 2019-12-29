#include "gfx.h"

#include <cmath>
#include <iostream>

#include "camera.cc"
#include "entity.cc"
#include "renderer/renderer.cc"

namespace gfx
{
CreateProjectionFunctor* _custom_projection = nullptr;

void
SetProjection(CreateProjectionFunctor* projection)
{
  _custom_projection = projection;
}

bool
Initialize()
{
  int window_result = window::Create("Space", 800, 800);
  std::cout << "window create result: " << window_result << std::endl;
  return rgg::Initialize();
}

void
RenderTriangles()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (NO_ENTITY(i)) break;
    if (NO_COMPONENT(i, triangle)) continue;

    Entity* ent = &game_entity[i];
    rgg::RenderTriangle(ent->transform.position, ent->transform.scale,
                        ent->transform.orientation, ent->triangle.color);
  }
}

void
RenderRectangles()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (NO_ENTITY(i)) break;
    if (NO_COMPONENT(i, rectangle)) continue;

    Entity* ent = &game_entity[i];
    rgg::RenderRectangle(ent->transform.position, ent->transform.scale,
                         ent->transform.orientation, ent->rectangle.color);
  }
}

void
RenderLines()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (NO_ENTITY(i)) break;
    if (NO_COMPONENT(i, line)) continue;

    Entity* ent = &game_entity[i];
    rgg::RenderLine(ent->line.start, ent->line.end, ent->line.color);
  }
}

void
RenderGrids()
{
  for (int i = 0; i < MAX_ENTITY; ++i) {
    if (NO_ENTITY(i)) break;
    if (NO_COMPONENT(i, grid)) continue;

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

  window::SwapBuffers();
}

}  // namespace gfx
