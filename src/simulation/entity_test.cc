#include <cstdio>

#include "simulation/common.cc"

struct Entity {
  uint64_t id;
};

DECLARE_GAME_TYPE(Entity, 8);

Entity* ents[kMaxEntity + 1];

void
print_all()
{
  for (uint64_t i = 0; i <= kMaxEntity; ++i) {
    printf("%p %p \n", ents[i], &kEntity[i]);
  }
}

void
print_ids()
{
  for (uint64_t i = 0; i < kUsedEntity; ++i)

  {
    printf("%p %lu\n", &kEntity[i], kEntity[i].id);
  }
}

int
main()
{
  for (uint64_t i = 0; i <= kMaxEntity; ++i) {
    printf("%lu used\n", kUsedEntity);
    ents[i] = UseEntity();
    if (ents[i]) {
      ents[i]->id = i;
    }
  }

  print_all();

  for (uint64_t i = 0; i <= kMaxEntity / 2; ++i) {
    ReleaseEntity(kMaxEntity / 2);
  }

  print_ids();

  for (uint64_t i = 0; i < kMaxEntity / 2; ++i) {
    ReleaseEntity(0);
  }

  puts("-- None expected");
  print_ids();
  puts("--");

  puts("--Safe to call release too many times");
  ReleaseEntity(0);
  print_ids();

  return 0;
}

