#pragma once

#include "math/mat.h"
#include "math/vec.h"

namespace camera
{
// Move the camera to position.
void MoveTo(const math::Vec3f& position);

// Translate the camera.
void Translate(const math::Vec3f& translation);

// Aim the camera in a direction with no rotation.
void AimAt(const math::Vec3f& dir);

// Returns the current position of the camera.
math::Vec3f position();

// Return the cameras current view matrix.
math::Mat4f view_matrix();

}  // namespace camera
