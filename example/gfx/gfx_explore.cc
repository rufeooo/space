
#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"

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
  //return math::CreatePerspectiveMatrix<float>(sz.x, sz.y, .1f, 100.f, 67.f);
  return math::CreateOrthographicMatrix<float>(
      sz.x, 0.f, sz.y, 0.f, 0.0f, 0.f);
}

int
main(int argc, char** argv)
{
  if (!window::Create("Graphics Explore", 800, 800)) return 1;
  
  rgg::Initialize();

  rgg::SetCameraTransformMatrix(math::CreateIdentityMatrix<float, 4>());
  rgg::SetViewMatrix(math::CreateIdentityMatrix<float, 4>());

  // 29
  constexpr int kVertCount = 29;
  constexpr int kFloatCount = kVertCount * 3;
  GLfloat asteroid[kFloatCount] = {
      0.f, 1.6f, 0.f, 0.2f, 1.5f, 0.f, 0.4f, 1.6f, 0.f, 0.6f, 1.6f, 0.f,
      0.68f, 1.9f, 0.f, 1.1f, 1.8f, 0.f, 1.6f, 1.7f, 0.f, 1.8f, 0.9f, 0.f,
      2.3f, 0.3f, 0.f, 2.4f, -0.5f, 0.f, 2.f, -0.8f, 0.f, 1.5f, -1.1f, 0.f,
      0.7f, -1.f, 0.f, 0.5f, -1.1f, 0.f, 0.2f, -1.3f, 0.f, -0.3f, -1.4f, 0.f,
      -1.1f, -1.1f, 0.f, -1.3f, -0.6f, 0.f, -1.25f, -0.2f, 0.f, -1.5f, 0.5f, 0.f,
      -1.4f, 0.4f, 0.f, -1.65f, 1.f, 0.f, -1.6f, 1.3f, 0.f, -1.6f, 1.7f, 0.f,
      -1.4f, 1.9f, 0.f, -1.f, 2.05f, 0.f, -0.7f, 2.07f, 0.f, -0.65f, 2.2f, 0.f,
      -0.5f, 2.25f, 0.f};
  for (int i = 0; i < kFloatCount; ++i) asteroid[i] *= 50.f;
  rgg::Tag tag = rgg::CreateRenderable(kVertCount, asteroid, GL_LINE_LOOP);

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
    rgg::RenderTag(
        tag,
        math::Vec3f(0.f, 0.f, 0.f),
        math::Vec3f(1.f, 1.f, 1.f),
        math::Quatf(0.f, math::Vec3f(0.f, 0.f, 1.f)),
        math::Vec4f(1.f, 0.f, 0.f, 0.5f));
    rgg::RenderGrid(50.f, 50.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));
    rgg::RenderGrid(25.f, 25.f, math::Vec4f(0.207f, 0.317f, 0.360f, 0.55f));

    glClear(GL_COLOR_DEPTH_BUFFER_BIT);
    rgg::RenderText("Test", 10.f, 50.f, math::Vec4f(1.f, 1.f, 1.f, 1.f));

    window::SwapBuffers();
  }

  return 0;
}
