
#include <stdio.h>
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

#define PRINTF_COUNTER(type) printf("%d\n", __COUNTER__)

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

/*#define ECS_ENUM(ptr_type)         \
  int f_type = TYPE_ID(*ptr_type); \
  ptr_type = ecs_memory(f_type);*/
#define ECS_ENUM(ptr_type) ptr_type = ecs_memory(TYPE_ID(*ptr_type))

int t1()
{
  c1_t f;
  return TYPE_ID(f);
}
int t2()
{
  c2_t f;
  return TYPE_ID(f);
}

int
main()
{
  ecs_init();

  c1_t f;
  int i;
  c1_t* iter_f;
  ECS_ENUM(iter_f);
  c1_t* iter_end = iter_f + PAGE / sizeof(*iter_f) + 1;

  for (int i = 0; iter_f < iter_end; ++iter_f, ++i) {
    iter_f->x = i;
    iter_f->y = i * 10;
  }

  ECS_ENUM(iter_f);
  for (int i = 0; iter_f < iter_end; ++iter_f, ++i) {
    printf("[ %d x ] [ %d y ]\n", iter_f->x, iter_f->y);
  }

  printf("%d c1\n", TYPE_ID(f));
  printf("%d int\n", TYPE_ID(i));
  int arr[] = {[0] = 3};
  // PRINTF_COUNTER(c1);
  return 0;
}

