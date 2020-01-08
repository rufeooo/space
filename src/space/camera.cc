#include "camera.h"

#include "math/math.cc"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

typedef math::Mat4f CreateProjectionFunctor(void);

struct Camera {
  math::Vec3f position;
  math::Quatf orientation;
};

namespace camera
{
CreateProjectionFunctor* _custom_projection = nullptr;
static Camera kCamera;

void
Initialize()
{
  camera::MoveTo(math::Vec3f(400.f, 400.f, 0.f));
}

void
MoveTo(const math::Vec3f& position)
{
  kCamera.position = position;
}

void
Translate(const math::Vec3f& translation)
{
  kCamera.position += translation;
}

void
AimAt(const math::Vec3f& dir)
{
  kCamera.orientation.Set(0.f, dir);
}

math::Vec3f
position()
{
  return kCamera.position;
}

math::Mat4f
transform_matrix()
{
  return math::CreateTranslationMatrix(kCamera.position) *
         math::CreateRotationMatrix(kCamera.orientation);
}

math::Mat4f
view_matrix()
{
  return math::CreateViewMatrix<float>(kCamera.position, kCamera.orientation);
}

math::Vec2f
ScreenToWorldSpace(const math::Vec2f& screen_pos)
{
  auto dims = window::GetWindowSize();
  // Inner expression is orienting the click position to have (0,0) be the
  // middle of the screen.
  //
  // Transform matrix is orientating the click to take into consideration
  // camera rotation / scale / translation.
  return (transform_matrix() * math::Vec3f(screen_pos - dims / 2.f)).xy();
}

void
UpdateView()
{
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
}

}  // namespace camera
