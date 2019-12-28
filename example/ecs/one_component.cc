#include "ecs/common.cc"

struct One {
};

inline ecs::ComponentStorage<One> kECS;

#define ECS_USE(type) \
  kECS.Assign<type>(1); \
  kECS.Enumerate<type>([](ecs::Entity e, type one) {}); \
  kECS.Get<type>(1); \
  kECS.Remove<type>(1); \
  kECS.Clear<type>(); \
  kECS.Delete(1);

int
main()
{
  ECS_USE(One);

  return 0;
}
