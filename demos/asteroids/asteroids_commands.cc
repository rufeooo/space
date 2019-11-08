#include "asteroids_commands.h"

namespace asteroids {

namespace commands {

void Execute(uint8_t* command_bytes) {
  const asteroids::Command* command =
      asteroids::GetCommand((void*)command_bytes);
  if (command->create_player()) {
    Execute(*command->create_player());
  }
  if (command->create_projectile()) {
    Execute(*command->create_projectile());
  }
  if (command->create_asteroid()) {
    Execute(*command->create_asteroid());
  }
  if (command->acknowledge()) {
  }
}

void Execute(const asteroids::CreatePlayer& create_player) {
}

void Execute(const asteroids::CreateProjectile& create_player) {
}

void Execute(const asteroids::CreateAsteroid& create_player) {
}

}

}
