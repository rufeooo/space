#pragma once

#include "math/math.cc"

#include "ecs/common.cc"

struct DestinationComponent {
  math::Vec2f position;
};

// Add new game types here.
inline ecs::ComponentStorage<DestinationComponent, RectangleComponent,
                             TransformComponent, TriangleComponent,
                             LineComponent, GridComponent>
    kECS;
