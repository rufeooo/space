
#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"

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
  if (!window::Create("Graphics Explore", 800, 800)) return 1;

  rgg::SetProjectionMatrix(PerspectiveProjection());
  rgg::SetViewMatrix(math::CreateIdentityMatrix<float, 4>());
  rgg::SetCameraTransformMatrix(math::CreateIdentityMatrix<float, 4>());
  rgg::SetViewport(window::GetWindowSize());

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}

    rgg::RenderLine(
        math::Vec3f(-500.f, 0.f, 0.f),
        math::Vec3f(500.f, 0.f, 0.f),
        math::Vec4f(1.f, 0.f, 0.f, 0.75f));

    rgg::RenderLine(
        math::Vec3f(0.f, -500.f, 0.f),
        math::Vec3f(0.f, 500.f, 0.f),
        math::Vec4f(0.f, 1.f, 0.f, 0.75f));

    window::SwapBuffers();
  }

  return 0;
}
