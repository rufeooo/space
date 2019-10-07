#pragma once

#include <thread>

#include "message_queue.h"
#include "network.h"

namespace network {

namespace client {

std::thread Create(const char* hostname, const char* port,
                   MessageQueue* outgoing_message_queue,
                   MessageQueue* incoming_message_queue);

}  // client

}  // network
