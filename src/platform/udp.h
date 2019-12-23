
extern "C" {
extern int udp_errno;
}

struct Udp4 {
  // File descriptor for socket
  int socket;
  // Family, address, port of destination
  char socket_address[16];
};


#ifdef __APPLE__
// TODO: Verify this value is correct.
#define MSG_NOSIGNAL 0x4000
#endif
