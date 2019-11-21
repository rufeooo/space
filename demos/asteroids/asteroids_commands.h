#pragma once

#include <functional>

#include "protocol/asteroids_commands_generated.h"
#include "ecs/entity.h"

namespace asteroids {

namespace commands {

void Execute(asteroids::CreatePlayer& create_player);
void Execute(asteroids::CreateProjectile& create_projectile);
void Execute(asteroids::CreateAsteroid& create_asteroid);
void Execute(asteroids::DeleteEntity& delete_entity);
void Execute(asteroids::Input& input);

}

}
