#include "gfx.h"

#include <cmath>
#include <iostream>

#include "renderer/renderer.cc"
#include "camera.cc"
#include "ecs.cc"

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
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          TriangleComponent& tri) {
        rgg::RenderTriangle(transform.position, transform.scale,
                            transform.orientation, tri.color);
      });
}

void
RenderRectangles()
{
  // Draw all rectangles.
  kECS.Enumerate<TransformComponent, RectangleComponent>(
      [&](ecs::Entity entity, TransformComponent& transform,
          RectangleComponent& rect) {
        rgg::RenderRectangle(transform.position, transform.scale,
                             transform.orientation, rect.color);
      });
}

void
RenderLines()
{
  kECS.Enumerate<LineComponent>([&](ecs::Entity entity, LineComponent& line) {
    rgg::RenderLine(line.start, line.end, line.color);
  });
}

void
RenderGrids()
{
  kECS.Enumerate<GridComponent>([&](ecs::Entity entity, GridComponent& grid) {
    rgg::RenderGrid(grid.width, grid.height, grid.color);
  });
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
