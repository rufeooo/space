
#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "space/camera.cc"

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
  return math::CreateOrthographicMatrix<float>(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
}

int
main(int argc, char** argv)
{
  if (!window::Create("Graphics Explore", 800, 800)) return 1;

  camera::Translate(math::Vec3f(-400.f, -400.f, 0.0f));
  camera::AimAt(math::Vec3f(0.f, 0.f, -1.f));
  
  rgg::Initialize();

  rgg::SetCameraTransformMatrix(camera::transform_matrix());
  rgg::SetViewMatrix(camera::view_matrix());

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}

    rgg::SetProjectionMatrix(PerspectiveProjection());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::RenderLine(
        math::Vec3f(-500.f, 0.f, 0.f),
        math::Vec3f(500.f, 0.f, 0.f),
        math::Vec4f(1.f, 0.f, 0.f, 0.75f));

    rgg::RenderLine(
        math::Vec3f(0.f, -500.f, 0.f),
        math::Vec3f(0.f, 500.f, 0.f),
        math::Vec4f(0.f, 1.f, 0.f, 0.75f));
   
    rgg::RenderGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));

    glClear(GL_DEPTH_BUFFER_BIT);
    ui::Text("Testing", 0.5f, 0.f, math::Vec4f());

    window::SwapBuffers();
  }

  return 0;
}
