#include <cstdio>

#include "common.cc"

struct Command {
  uint64_t action;
};

struct Packet {
  uint64_t turn;
  char buffer[8];
};

// test n
DECLARE_GAME_QUEUE(Command, 8);
// test n+1
DECLARE_GAME_QUEUE(Packet, 8);

int
main()
{
  // Add kMax to test failed push
  for (uint64_t i = 0; i < kMaxCommand; ++i) {
    uint64_t before = kWriteCommand;
    PushCommand(Command{i});
    uint64_t after = kWriteCommand;
    printf("Push Command %lu, index %lu->%lu\n", i, before, after);
  }

  // Pop kMax to test failed pop
  for (uint64_t i = 0; i < kMaxCommand; ++i) {
    uint64_t before = kReadCommand;
    Command c = PopCommand();
    uint64_t after = kReadCommand;
    printf("Pop Command, action %lu, index %lu->%lu\n", c.action, before,
           after);
  }

  return 0;
}
