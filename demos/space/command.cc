#include "command.h"

#include "ecs.h"
#include "components/common/transform_component.h"

namespace command {

void Execute(const Move& move) {
  auto* transform = kECS.Get<TransformComponent>(move.entity_id);
  transform->position = math::Vec3f(move.position.x, move.position.y, 0.f);
}

}
