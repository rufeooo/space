#pragma once

#include "components.h"
#include "components/common/transform_component.h"
#include "components/rendering/grid_component.h"
#include "components/rendering/rectangle_component.h"
#include "components/rendering/triangle_component.h"
#include "components/rendering/line_component.h"
#include "ecs/ecs.h"

// Add new game types here.
inline ecs::ComponentStorage<DestinationComponent, RectangleComponent,
                             TransformComponent, TriangleComponent,
                             LineComponent, GridComponent>
    kECS;
