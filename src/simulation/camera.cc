
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
  // Vector of movement for the camera
  //   Useful for stateful translation (i.e. the duration of keypress)
  math::Vec2f motion;
};

namespace camera
{
void
Update(Camera* cam)
{
  cam->position += cam->motion;
}

void
SetView(const Camera* cam, math::Mat4f* view)
{
  *view = math::CreateViewMatrix(cam->position, cam->orientation);
}

// Transform matrix is orientating the click to take into consideration
// camera rotation / scale / translation.
math::Vec3f
ScreenToWorldSpace(const Camera* cam, const math::Vec3f screen)
{
  math::Mat4f camera_transform = math::CreateRotationMatrix(cam->orientation) *
                                 math::CreateTranslationMatrix(cam->position);
  math::Vec3f ret = camera_transform * screen;

  return ret;
}

void
InitialCamera(Camera* cam)
{
  cam->position = math::Vec3f(400.f, 400.f, 0.f);
  // cam->orientation.Set(45.f, math::Vec3f(0.f, 0.f, 1.f));
}

void
OrientCamera(const math::Vec3f& dir, Camera* cam)
{
  cam->orientation.Set(0.f, dir);
}

}  // namespace camera
