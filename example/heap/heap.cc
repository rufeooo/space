#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>

bool
compare(uint64_t lhs, uint64_t rhs)
{
  return lhs < rhs;
}

void
push_heap(uint64_t* first, size_t hole_index, size_t top_index, uint64_t value)
{
  size_t parent = (hole_index - 1) / 2;
  while (hole_index > top_index && compare(*(first + parent), value)) {
    *(first + hole_index) = *(first + parent);
    hole_index = parent;
    parent = (hole_index - 1) / 2;
  }
  *(first + hole_index) = value;
}

void
adjust_heap(uint64_t* first, size_t hole_index, size_t len, uint64_t value)
{
  const size_t top_index = hole_index;
  size_t second_child = hole_index;
  while (second_child < (len - 1) / 2) {
    second_child = 2 * (second_child + 1);
    if (compare(*(first + second_child), *(first + (second_child - 1))))
      second_child--;
    *(first + hole_index) = *(first + second_child);
    hole_index = second_child;
  }
  if ((len & 1) == 0 && second_child == (len - 2) / 2) {
    second_child = 2 * (second_child + 1);
    *(first + hole_index) = *(first + (second_child - 1));
    hole_index = second_child - 1;
  }
  push_heap(first, hole_index, top_index, value);
}

void
make_heap(uint64_t* first, uint64_t* last)
{
  if (last - first < 2) return;

  const size_t len = last - first;
  size_t parent = (len - 2) / 2;
  while (true) {
    uint64_t value = *(first + parent);
    adjust_heap(first, parent, len, value);
    if (parent == 0) return;
    parent--;
  }
}

uint64_t
pop_heap(uint64_t* first, uint64_t* last)
{
  if (last - first <= 1) return *first;

  uint64_t result = *first;
  --last;
  uint64_t value = *last;
  adjust_heap(first, size_t(0), size_t(last - first), value);

  return result;
}

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

int
main()
{
  uint64_t test_array[] = {
      571231283, 318, 188, 97123, 947213, 523213, 5713, 977123, 853213,
      5712,      38,  187, 971,   9413,   5223,   513,  9712,   53213,
  };
  make_heap(test_array, test_array + ARRAY_LENGTH(test_array));

  for (int i = 0; i < ARRAY_LENGTH(test_array); ++i) {
    printf("%lu ", test_array[i]);
  }
  puts("");
  puts("Press return");
  getc(stdin);

  uint64_t* first = test_array;
  uint64_t* last = test_array + ARRAY_LENGTH(test_array);
  uint64_t val = ~0ul;
  while (last > first) {
    uint64_t next_val = pop_heap(first, last);
    assert(next_val < val);
    val = next_val;
    printf("%lu ", val);
    --last;
  }
  puts("");
}

