#pragma once

#include <thread>

#include "network.h"

namespace network {

namespace client {

std::thread Create(const char* hostname, const char* port);

}  // client

}  // network
