#pragma once

namespace ecs {

template <typename... Components>
class System {
 public:
  virtual void Run(Components&...) = 0;
}

}
