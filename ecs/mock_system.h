#pragma once

#include "gmock/gmock.h"
#include "system.h"

namespace ecs {

template <typename... Components>
class MockSystem : public System<Components...> {
 public:
  MOCK_METHOD(void,
              OnEntity,
              (Entity entity, Components&...),
              (override));
};

}  // ecs
