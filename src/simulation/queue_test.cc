#include <cassert>
#include <cstdio>

#include "common.cc"

struct Command {
  uint64_t action;
};

struct Packet {
  uint64_t turn;
  char buffer[8];
};

// declare 1 game queue
DECLARE_GAME_QUEUE(Command, 8);
// declare 1+1 game queue
DECLARE_GAME_QUEUE(Packet, 8);

int
main()
{
  // Add kMax+1 to test failed push
  for (uint64_t i = 0; i < kMaxCommand + 1; ++i) {
    uint64_t before = kWriteCommand;
    PushCommand(Command{i});
    uint64_t after = kWriteCommand;
    printf("Push Command %lu, index %lu->%lu\n", i, before, after);
  }

  // Pop kMax+1 to test failed pop
  for (uint64_t i = 0; i < kMaxCommand + 1; ++i) {
    uint64_t before = kReadCommand;
    Command c = PopCommand();
    uint64_t after = kReadCommand;
    printf("Pop Command, action %lu, index %lu->%lu\n", c.action, before,
           after);
  }

  // Many game iterations of queueing
  for (uint64_t overflows = 0; overflows < 4; ++overflows) {
    constexpr uint64_t limit = UINT64_MAX / kMaxCommand;
    printf("performing %lu iterations\n", limit);
    for (uint64_t i = 0; i < limit + 1; ++i) {
      for (uint64_t j = 0; j < kMaxCommand; ++j) {
        PushCommand(Command{j + 1});
      }
      for (uint64_t j = 0; j < kMaxCommand; ++j) {
        assert(PopCommand().action == j + 1);
      }
    }

    printf("%lu final writes, %lu final reads (expect unsigned overflow)\n",
           kWriteCommand, kReadCommand);
  }

  PushCommand(Command{42});
  assert(PopCommand().action == 42);
  PushCommand(Command{1});
  assert(PopCommand().action == 1);

  return 0;
}
