#include "asteroids_serialize.h"

namespace asteroids {

std::vector<uint8_t> Serialize(
    asteroids::CreatePlayer& create_player) {
  flatbuffers::FlatBufferBuilder fbb;
  auto command = asteroids::CreateCommand(
      fbb, 0, &create_player);
  fbb.Finish(command);
  auto detached_buffer = fbb.Release();
  return std::vector<uint8_t>(
      detached_buffer.data(),
      detached_buffer.data() + detached_buffer.size());
}

std::vector<uint8_t> Serialize(
    asteroids::CreateProjectile& create_projectile) {
  flatbuffers::FlatBufferBuilder fbb;
  auto command = asteroids::CreateCommand(
      fbb, 0, 0, &create_projectile);
  fbb.Finish(command);
  auto detached_buffer = fbb.Release();
  return std::vector<uint8_t>(
      detached_buffer.data(),
      detached_buffer.data() + detached_buffer.size());
}

std::vector<uint8_t> Serialize(
    asteroids::CreateAsteroid& create_asteroid) {
  flatbuffers::FlatBufferBuilder fbb;
  auto command = asteroids::CreateCommand(
      fbb, 0, 0, 0, &create_asteroid);
  fbb.Finish(command);
  auto detached_buffer = fbb.Release();
  return std::vector<uint8_t>(
      detached_buffer.data(),
      detached_buffer.data() + detached_buffer.size());
}

}
