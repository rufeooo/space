#include <stdint.h>

#include <stdio.h>

#include "platform.cc"

void*
whee(ThreadInfo* t)
{
  for (int i = 0; i < 1000; ++i) {
    printf("thread fun ");
  }

  t->return_value = 500;
  return &t->return_value;
}

int
main()
{
  printf("%lu\n", sizeof(pthread_t));
  static ThreadInfo t;
  thread_create(&t, whee);

  thread_join(&t);
  printf("ret %lu\n", t.return_value);
  return 0;
}
