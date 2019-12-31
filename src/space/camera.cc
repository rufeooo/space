#include "camera.h"

#include "math/math.cc"
#include "platform/platform.cc"

namespace camera
{
struct Camera {
  math::Vec3f position;
  math::Quatf orientation;
};

static Camera kCamera;

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
GetClickInWorldSpace(const math::Vec2f& click_pos)
{
  auto dims = window::GetWindowSize();
  // Inner expression is orienting the click position to have (0,0) be the
  // middle of the screen.
  //
  // Transform matrix is orientating the click to take into consideration
  // camera rotation / scale / translation.
  return (transform_matrix() * math::Vec3f(click_pos - dims / 2.f)).xy();
}

}  // namespace camera
