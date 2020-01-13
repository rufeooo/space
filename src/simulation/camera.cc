
// clang-format off
/*
 * While painting "broad strokes" of the game it it suitable to place camera into the simulation:
 *   ( this means player cameras are network synchronized )
 *   - platform inputs are dispatched unconditionally, and therefore easy to reason about
 *   - gameplay dev should be aware of the latency of inputs
 *   - increases information sharing, which is in the spirit of co-op
 *
 * The shipping game is unlikely to maintain this behavior. Update this list as we go:
 *   - Camera movement is a common activity, it should not feel "sluggish" due to network dependencies
 *   - How useful is it to see where other player's are looking? Does it add a liveliness to the world?
 *   - Window size changes the camera, so how do we handle initialization or a window resize during pause?
 * 
 */
// clang-format on
#include "math/math.cc"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

struct Camera {
  math::Vec3f position;
  math::Quatf orientation;
  math::Mat4f projection;
  // Useful for stateful translation (i.e. continue change for duration of
  // keypress)
  math::Vec2f translation;
};

namespace camera
{
void
ConfigureObserver(const Camera* cam, rgg::Observer* observer)
{
  observer->camera_transform = math::CreateTranslationMatrix(cam->position) *
                               math::CreateRotationMatrix(cam->orientation);
  observer->view =
      math::CreateViewMatrix<float>(cam->position, cam->orientation);
  observer->projection = cam->projection;
}

math::Vec2f
ScreenToWorldSpace(const Camera* cam, const math::Vec2f& screen_pos)
{
  auto dims = window::GetWindowSize();
  // Inner expression is orienting the click position to have (0,0) be the
  // middle of the screen.
  //
  // Transform matrix is orientating the click to take into consideration
  // camera rotation / scale / translation.

  math::Mat4f camera_transform = math::CreateTranslationMatrix(cam->position) *
                                 math::CreateRotationMatrix(cam->orientation);
  return (camera_transform * math::Vec3f(screen_pos - dims * 0.5f)).xy();
}

void
InitialCamera(Camera* cam, math::Vec2f dims)
{
  cam->position = math::Vec3f(400.f, 400.f, 0.f);
  cam->projection = math::CreateOrthographicMatrix<float>(
      dims.x, 0.f, dims.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
}

void
OrientCamera(const math::Vec3f& dir, Camera* cam)
{
  cam->orientation.Set(0.f, dir);
}

}  // namespace camera
