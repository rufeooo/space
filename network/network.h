#if defined(_WIN32)
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <errno.h>
  #define SOCKET int
  #define INVALID_SOCKET (-1)
#endif

#include <stdio.h>

namespace network {

// On unix - Does nothing.
// On windows - Calls WSAStartup.
bool SocketInit();

// On unix - Checks that s >= 0
// On windows - Checks that s != INVALID_SOCKET
bool SocketIsValid(SOCKET s);

// On unix - Calls close(s)
// On windows - Calls closesocket(s)
bool SocketClose(SOCKET s);

// On unix - Returns errno
// On windows - Returns WSAGetLastError()
int SocketErrno();

}
