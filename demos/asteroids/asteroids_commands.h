#pragma once

#include <functional>

#include "protocol/asteroids_commands_generated.h"
#include "ecs/entity.h"

namespace asteroids {

namespace commands {

void Execute(uint8_t* command_bytes);

void Execute(asteroids::CreatePlayer& create_player,
             bool is_remote=false);
void Execute(asteroids::CreateProjectile& create_projectile,
             bool is_remote=false);
void Execute(asteroids::CreateAsteroid& create_asteroid,
             bool is_remote=false);
void Execute(asteroids::DeleteEntity& delete_entity,
             bool is_remote=false);
void Execute(asteroids::UpdateTransform& update_transform,
             bool is_remote=false);
void Execute(asteroids::UpdateInput& update_input,
             bool is_remote=false);

}

}
