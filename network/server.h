#pragma once

#include <thread>
#include <array>

#include "message_queue.h"
#include "network.h"

namespace network {

namespace server {

constexpr int kMaxClients = 10;

std::thread Create(const char* port,
                   std::array<OutgoingMessageQueue, kMaxClients>*
                      outgoing_message_queue,
                   IncomingMessageQueue* incoming_message_queue);

}  // server

}  // network
