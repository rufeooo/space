
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
 */
#include "math/math.cc"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

typedef math::Mat4f CreateProjectionFunctor(void);

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
CreateProjectionFunctor* _custom_projection = nullptr;

void
AimAt(const math::Vec3f& dir, Camera* cam)
{
  cam->orientation.Set(0.f, dir);
}

math::Mat4f
transform_matrix(const Camera* cam)
{
  return math::CreateTranslationMatrix(cam->position) *
         math::CreateRotationMatrix(cam->orientation);
}

math::Mat4f
view_matrix(const Camera* cam)
{
  return math::CreateViewMatrix<float>(cam->position, cam->orientation);
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
  return (transform_matrix(cam) * math::Vec3f(screen_pos - dims / 2.f)).xy();
}

void
InitialCamera(Camera* cam)
{
  cam->position = math::Vec3f(400.f, 400.f, 0.f);

  // TODO: don't assume every camera uses local window size
  math::Vec2f dims = window::GetWindowSize();
  cam->projection = math::CreateOrthographicMatrix<float>(
      dims.x, 0.f, dims.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
}

void
CustomCamera(Camera* cam, math::Mat4f custom_projection)
{
  cam->position = math::Vec3f(400.f, 400.f, 0.f);
  cam->projection = custom_projection;
}

}  // namespace camera
