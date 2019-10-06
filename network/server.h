#pragma once

#include <thread>

#include "message_queue.h"
#include "network.h"

namespace network {

namespace server {

std::thread Create(const char* port,
                   MessageQueue* outgoing_message_queue,
                   MessageQueue* incoming_message_queue);

}  // server

}  // network
