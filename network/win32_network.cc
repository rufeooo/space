#include "network.h"

namespace network
{
bool
SocketInit()
{
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d)) {
    return false;
  }
  return true;
}

bool
SocketIsValid(SOCKET s)
{
  return s != INVALID_SOCKET;
}

bool
SocketClose(SOCKET s)
{
  closesocket(s);
  return true;
}

int
SocketErrno()
{
  return WSAGetLastError();
}

}  // namespace network
