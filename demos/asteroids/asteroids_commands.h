#pragma once

#include "protocol/asteroids_commands_generated.h"

namespace asteroids {

namespace commands {

void Execute(uint8_t* command_bytes);
void Execute(const asteroids::CreatePlayer& create_player);
void Execute(const asteroids::CreateProjectile& create_player);
void Execute(const asteroids::CreateAsteroid& create_player);

}

}
