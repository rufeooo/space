#include "math/mat.h"
#include "math/quat.h"
#include "math/vec.h"

// rgg for render go-go!

namespace rgg {

bool Initialize();

void SetProjectionMatrix(const math::Mat4f& projection);

void SetViewMatrix(const math::Mat4f& view);

void SetCameraTransformMatrix(const math::Mat4f& camera_transform);

void RenderTriangle(const math::Vec3f& position,
                    const math::Vec3f& scale,
                    const math::Quatf& orientation,
                    const math::Vec4f& color);

void RenderRectangle(const math::Vec3f& position,
                     const math::Vec3f& scale,
                     const math::Quatf& orientation,
                     const math::Vec4f& color);

void RenderLine(const math::Vec3f& start,
                const math::Vec3f& end,
                const math::Vec4f& color);

void RenderGrid(float width, float height, const math::Vec4f& color);

}
