#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"

void
AlignToGrid(v2f grid, math::Rectf* world)
{
  float x_align = fmodf(world->min.x, grid.x);
  float y_align = fmodf(world->min.y, grid.y);

  world->min.x -= x_align + grid.x;
  world->min.y -= y_align + grid.y;
}

void
RenderGrid()
{
  auto dims = window::GetWindowSize();
  math::Rectf world;
  world.min = {-dims.x / 2.f, -dims.y / 2.f};
  world.max = {dims.x / 2.f, dims.y / 2.f};

  const v2f grid2(50.f, 50.f);
  math::Rectf world2 = world;
  AlignToGrid(grid2, &world2);
  rgg::RenderGrid(grid2, world2, v4f(0.207f, 0.317f, 0.360f, 0.60f));

  const v2f grid1(25.f, 25.f);
  math::Rectf world1 = world;
  AlignToGrid(grid1, &world1);
  rgg::RenderGrid(grid1, world1, v4f(0.050f, 0.215f, 0.050f, 0.55f));
}

math::Mat4f PerspectiveProjection(v2f dims)
{
  return math::Ortho(
      dims.x, 0.f, dims.y, 0.f, 0.0f, 0.f);
}

int
main(int argc, char** argv)
{
  if (!window::Create("Graphics Explore", 800, 800)) return 1;
  
  rgg::Initialize();

  rgg::Texture texture = rgg::CreateTexture2D(GL_RGB, 256, 256, NULL);

  rgg::BeginRenderTo(texture);

  glClear(GL_COLOR_BUFFER_BIT);

  // This projection orients the bottom left of the screen to be origin.
  // This makes the texture have consistent axis with opengl as well as screen.
  rgg::GetObserver()->projection =
      math::Ortho2(
          texture.width, 0.0f, texture.height, 0.0f, 0.0f, 0.0f);

  rgg::RenderRectangle(
      math::Rect(0.0f, 0.0f, 50.0f, 50.0f),
      v4f(1.0f, 0.0f, 0.0f, 1.0f));

  rgg::RenderRectangle(
      math::Rect(texture.width - 25.0f, texture.height - 25.0f, 25.0f, 25.0f),
      v4f(0.0f, 1.0f, 0.0f, 1.0f));

  rgg::RenderRectangle(
      math::Rect(texture.width - 10.0f, 0.0f, 10.0f, 10.0f),
      v4f(0.0f, 0.0f, 1.0f, 1.0f));

  rgg::EndRenderTo();

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

    rgg::GetObserver()->projection = PerspectiveProjection(window::GetWindowSize());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderGrid();

    rgg::RenderTexture(texture,
                       math::Rect(0.0f, 0.0f, 256.0f, 256.0f),
                       math::Rect(0.0f, 0.0f, 256.0f, 256.0f));

    // Renders the red rectangle from the texture sheet.
    rgg::RenderTexture(texture,
                       math::Rect(0.0f, 0.0f, 50.0f, 50.0f),
                       math::Rect(-100.0f, -200.0f, 50.0f, 50.0f));

    // Renders the green rectangle from the texture sheet.
    rgg::RenderTexture(texture,
                       math::Rect(texture.width - 25.0f,
                                  texture.height - 25.0f,
                                  25.0f, 25.0f),
                       math::Rect(-25.0f, -200.0f, 25.0f, 25.0f));

    // Renders the blue rectangle from the texture sheet.
    rgg::RenderTexture(texture,
                       math::Rect(texture.width - 10.0f, 0.0f, 10.0f, 10.0f),
                       math::Rect(20.0f, -200.0f, 10.0f, 10.0f));


    rgg::RenderText("Texture", 50.0f, 100.0f, v4f(1.f, 1.f, 1.f, 1.f));

    window::SwapBuffers();
  }

  return 0;
}
