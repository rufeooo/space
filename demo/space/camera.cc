#include "camera.h"

#include "math/mat_ops.h"
#include "math/quat.h"

namespace camera {

struct Camera {
  math::Vec3f position;
  math::Quatf orientation;
};

static Camera kCamera;

void MoveTo(const math::Vec3f& position) {
  kCamera.position = position;
}

void Translate(const math::Vec3f& translation) {
  kCamera.position += translation;
}

void AimAt(const math::Vec3f& dir) {
  kCamera.orientation.Set(0.f, dir);
}

math::Vec3f position() {
  return kCamera.position;
}

math::Mat4f view_matrix() {
  return math::CreateViewMatrix<float>(
      kCamera.position, kCamera.orientation);
}

}
