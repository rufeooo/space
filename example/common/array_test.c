#include "common/common.h"

#include "common/array.c"

typedef struct entityS {
  uint64_t id;
} Entity;

DECLARE_ARRAY(Entity, 8);

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
    CompressEntity(kMaxEntity / 2);
  }

  print_ids();

  for (uint64_t i = 0; i < kMaxEntity / 2; ++i) {
    CompressEntity(0);
  }

  puts("-- None expected");
  print_ids();
  puts("--");

  puts("--Safe to call release too many times");
  CompressEntity(0);
  print_ids();

  return 0;
}
