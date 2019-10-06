#include <gflags/gflags.h>

#include "network/network.h"

DEFINE_string(hostname, "127.0.0.1", "hostname");
DEFINE_string(port, "8080", "port");
DEFINE_string(message, "Hello!", "message to send");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (!network::SocketInit()) {
    printf("Failed to initialize...\n\n");
    return 1;
  }

  return 0;
}
