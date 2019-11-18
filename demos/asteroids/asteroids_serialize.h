#pragma once

#include <vector>

#include "protocol/asteroids_commands_generated.h"

namespace asteroids {

std::vector<uint8_t> Serialize(
    asteroids::CreatePlayer& create_player);
std::vector<uint8_t> Serialize(
    asteroids::CreateProjectile& create_projectile);
std::vector<uint8_t> Serialize(
    asteroids::CreateAsteroid& create_asteroid);

}
