#include "network.h"

namespace network {

bool SocketInit() {
  return true;
}

bool SocketIsValid(SOCKET s) {
  return s >= 0;
}

bool SocketClose(SOCKET s) {
  close(s);
  return true;
}

int SocketErrno() {
  return errno;
}

}
