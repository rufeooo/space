#pragma once

#include "network.h"

namespace network {

namespace client {

// Call when message is received from the server.
typedef void (*OnMsgReceived)(uint8_t*, int);

void Setup(OnMsgReceived on_msg_received_callback);

// Start client and begin receiving messages from the hostname.
bool Start(const char* hostname, const char* port);

void Stop();

void Send(uint8_t* buffer, int size);

}  // client

}  // network
