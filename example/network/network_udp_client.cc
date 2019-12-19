#include <iostream>
#include <thread>

#include <gflags/gflags.h>

#include "network/client.h"

DEFINE_string(hostname, "127.0.0.1",
              "If provided will connect to a game server. Will play "
              "the game singleplayer otherwise.");

DEFINE_string(port, "9843", "Port for this application.");

void
SendGarbage()
{
}

int
main(int argc, char** argv)
{
  return 0;
}
