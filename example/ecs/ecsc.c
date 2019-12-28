
#if 0
#include <stdio.h>
#endif
#include <stdlib.h>

typedef struct {
  int x;
  int y;
} c1_t;
typedef struct {
  int z;
} c2_t;

#define ECS_TYPE(V, ID) \
  V:                    \
  ID

// clang-format off
#define TYPE_ID(X) _Generic((X), \
    default: -1, \
    ECS_TYPE(c1_t, 0), \
    ECS_TYPE(c2_t, 1) \
)
#define MAX_ECS_TYPE 2
// clang-format on
#define PAGE (4 * 1024)

typedef struct {
  void* memory;
} ecstype_t;
ecstype_t typeinfo[MAX_ECS_TYPE];

void
ecs_init()
{
  for (int i = 0; i < MAX_ECS_TYPE; ++i) {
    typeinfo[i].memory = malloc(PAGE);
  }
}

void
ecs_shutdown()
{
  for (int i = 0; i < MAX_ECS_TYPE; ++i) {
    free(typeinfo[i].memory);
    typeinfo[i].memory = 0;
  }
}

void*
ecs_memory(int type_id)
{
  return typeinfo[type_id].memory;
}

#define ECS_GET(ptr_type, index) \
  ptr_type = ecs_memory(TYPE_ID(*ptr_type)) + index
#define ECS_ENUM_END(ptr_type) \
  ptr_type = ecs_memory(TYPE_ID(*ptr_type)) + PAGE / sizeof(*ptr_type) + 1;

// c1_t* iter_end = iter_f + PAGE / sizeof(*iter_f) + 1;
int
t1()
{
  c1_t f;
  return TYPE_ID(f);
}
int
t2()
{
  c2_t f;
  return TYPE_ID(f);
}

int
main()
{
  ecs_init();

  c1_t f = {.x = 1, .y = 2};
  int i;
  c1_t* iter_f;
  c1_t* iter_end;
  ECS_GET(iter_f, 0);
  *iter_f = f;
  ECS_GET(iter_f, 0);
  ECS_ENUM_END(iter_end);
  for (int i = 0; iter_f < iter_end; ++iter_f, ++i) {
    iter_f->x = i;
    iter_f->y = i * 10;
  }
  ECS_GET(iter_f, 0);
#if 0
  printf("[ %d x ] [ %d y ]\n", iter_f->x, iter_f->y);
#endif
  ECS_GET(iter_f, 0);
  *iter_f = (c1_t){0};
  ECS_GET(iter_f, 0);
#if 0
  printf("[ %d x ] [ %d y ]\n", iter_f->x, iter_f->y);
#endif

  ECS_GET(iter_f, 0);
  for (int i = 0; iter_f < iter_end; ++iter_f, ++i) {
#if 0
    printf("[ %d x ] [ %d y ]\n", iter_f->x, iter_f->y);
#endif
  }

#if 0
  printf("%d c1\n", TYPE_ID(f));
  printf("%d int\n", TYPE_ID(i));
#endif
  int arr[] = {[0] = 3};
  return 0;
}

