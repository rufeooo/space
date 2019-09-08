#pragma once

#include "ecs.h"

namespace ecs {

template <typename... Components>
class System {
 public:
  virtual void OnEntity(Entity entity, Components&...) = 0;

  void Run() {
    ecs::Enumerate<Components...>([this](auto&... params) {
      OnEntity(params...);
    }); 
  }
};

}
