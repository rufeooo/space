#include "ecs/common.cc"

struct One {
};
struct Two {
};
struct Three {
};
struct Four {
};

inline ecs::ComponentStorage<One, Two, Three, Four> kECS;

#define ECS_USE(type)                                   \
  kECS.Assign<type>(1);                                 \
  kECS.Enumerate<type>([](ecs::Entity e, type one) {}); \
  kECS.Get<type>(1);                                    \
  kECS.Remove<type>(1);                                 \
  kECS.Clear<type>();                                   \
  kECS.Delete(1);

int
main()
{
  ECS_USE(One);
  ECS_USE(Two);
  ECS_USE(Three);
  ECS_USE(Four);

  return 0;
}
