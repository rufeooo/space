#pragma once

#include "ecs/ecs.h"
#include "components/common/transform_component.h"
#include "components/rendering/triangle_component.h"

// Add new game types here.
inline ecs::ComponentStorage<
        TransformComponent,
        TriangleComponent> kECS;
