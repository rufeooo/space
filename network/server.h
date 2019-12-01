#pragma once

#include "network.h"

namespace network {

namespace server {

// The following callbacks are called on a seperate thread.

// Called when a client connects with a client_id.
typedef void (*OnClientConnected)(int);

// Called when a message is received from a client_id.
typedef void (*OnMsgReceived)(int, uint8_t*, int);

void Setup(
    OnClientConnected on_client_connected_callback,
    OnMsgReceived on_msg_received_callback);

// Start the server and bind it
bool Start(const char* port);

// Shutdown the server connection and terminates the server thread.
void Stop();

// Send a message to a specific client.
void Send(int client_id, uint8_t* buffer, int size);

}  // server

}  // network
