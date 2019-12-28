#include "ecs/common.cc"

struct c1 {
};
struct c2 {
};
struct c3 {
};
struct c4 {
};
struct c5 {
};
struct c6 {
};
struct c7 {
};
struct c8 {
};

inline ecs::ComponentStorage<c1, c2, c3, c4, c5, c6, c7, c8> kECS;

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
  ECS_USE(c1);
  ECS_USE(c2);
  ECS_USE(c3);
  ECS_USE(c4);
  ECS_USE(c5);
  ECS_USE(c6);
  ECS_USE(c7);
  ECS_USE(c8);

  return 0;
}
