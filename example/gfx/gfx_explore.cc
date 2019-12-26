
#include "platform/platform.cc"
#include "space/camera.cc"
#include "gl/renderer.cc"
#include "space/gfx.cc"

// ORTHO:
//0.002500,0.000000,0.000000,-1.000000
//0.000000,0.002500,0.000000,-1.000000
//0.000000,0.000000,-2.000000,-0.000000
//0.000000,0.000000,0.000000,1.000000/
//
// PERSPECTIVE:
//1.510835,0.000000,0.000000,0.000000
//0.000000,1.510835,0.000000,0.000000
//0.000000,0.000000,-1.002002,-0.200200
//0.000000,0.000000,-1.000000,0.000000/

math::Mat4f PerspectiveProjection() {
  auto sz = window::GetWindowSize();
  return math::CreatePerspectiveMatrix<float>(sz.x, sz.y, 0.1f, 100.f, 67.f);
}

int
main(int argc, char** argv)
{
  if (!gfx::Initialize()) return 1;

  gfx::SetProjection(&PerspectiveProjection);

  camera::Translate(math::Vec3f(0.f, 0.f, 5.0f));
  // Aim it down the z axis.
  camera::AimAt(math::Vec3f(0.f, 0.f, -1.f));

  // x/y/z axis.
  auto* x = kECS.Assign<LineComponent>(1);
  x->start = math::Vec3f(-500.f, 0.f, 0.f);
  x->end = math::Vec3f(500.f, 0.f, 0.f);
  x->color = math::Vec4f(1.f, 0.f, 0.f, 0.75f);

  auto* y = kECS.Assign<LineComponent>(2);
  y->start = math::Vec3f(0.f, -500.f, 0.f);
  y->end = math::Vec3f(0.f, 500.f, 0.f);
  y->color = math::Vec4f(0.f, 1.f, 0.f, 0.75f);


  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    gfx::Render();
  }

  return 0;
}
