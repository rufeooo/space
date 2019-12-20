#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Input types:
// 1) key/button
// 2) Mouse Pointer
// ... tbd (joystick, touch screen, mouse wheel.. ?)
//
// There is a four stage translation from input to execution:
// (stage 1) Input Queue -> 
// (stage 2) Application processing into gameinput
// (stage 3) Network transmission
// (stage 4) Game processing of playerinput
//
// The data view of each stage is:
// (stage 1) Operating System dependent (mac/win/nix)
// (stage 2) Application: GameInputs
// (stage 3) Network: raw copy of GameInputs
// (stage 4) Game: gather/scatter of player inputs into game mutation
//
// The player perspective includes non-gameplay elements:
// 1) Point of View: Camera
// 2) Hardware Cursor
// ... tbd (UI)
//
// Gameplay example: (Player A: PA, Player B: PB)
// PA mousedown selects unit
// PB sees selection
// PB pressed 'M'
// 'M' down energizes transporter to beam medical officer
// PB clicks next to PA's seleted unit.
//
// Gameplay input proposal per frame 1+ events: [minimum 1 mpos per frame]
// <mpos> <button/key>
//
// The Game Server CAN inject empty frames.
// The format of an empty frame is simply a repetition of the last transmitted 
// mpos with an empty key.
//
int main()
{
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  
  if (s < 0) {
    printf("fail socket");
    exit(errno);
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5000);
  if (!inet_pton(AF_INET, "::ffff:192.168.86.240", &addr.sin_addr)) {
    printf("fail inet_pton");
    exit(errno);
  }

#define MAX_PACKET  64*1024
  char buffer[MAX_PACKET];
  int net_len;
  while (net_len < MAX_PACKET) {
    const char* cstr = "abcdefhijklmnopqrstuvwxyz";
    const unsigned cstr_len = strlen(cstr);
    memcpy(buffer+net_len, cstr, cstr_len);
    net_len+= cstr_len;
    if (sendto(s, buffer, net_len, 0,
        (const struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
      printf("fail sendto. net_len is %d", net_len);
      exit(errno);
    }
  }

  printf("net_len is %d\n", net_len);

  return 0;
}
