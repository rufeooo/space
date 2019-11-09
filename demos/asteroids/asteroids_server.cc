#include <array>
#include <thread>
#include <gflags/gflags.h>
#include <cassert>

#include "asteroids.h"
#include "asteroids_commands.h"
#include "asteroids_state.h"
#include "network/server.h"
#include "network/message_queue.h"
#include "protocol/asteroids_commands_generated.h"
#include "game/game.h"

DEFINE_string(port, "9845", "Port for this application.");
// TODO: Reimplement.
DEFINE_bool(headless, true, "Set to false to render game. Should be "
                            "used for debug only");

void ProcessIncomingCommands() {
  // Execute all commands received from clients.
  auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
  network::Message msg =
      connection_component->incoming_message_queue.Dequeue();
  while (msg.size != 0) {
    asteroids::commands::Execute(msg.data);
    msg = connection_component->incoming_message_queue.Dequeue();
  }
}

void ProcessOutgoingCommands() {
  auto& components = asteroids::GlobalGameState().components;
  auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
  components.Enumerate<component::ServerAuthoratativeComponent<
      component::TransformComponent>>([&](
          ecs::Entity entity, component::ServerAuthoratativeComponent<
            component::TransformComponent>){
    auto* transform_component =
        components.Get<component::TransformComponent>(entity);
    assert(transform_component != nullptr);
    // TODO: Make automatic conversions for some of these things...
    flatbuffers::FlatBufferBuilder fbb;
    auto& p = transform_component->position;
    auto& pp = transform_component->prev_position;
    auto& o = transform_component->orientation;
    asteroids::UpdateTransform update_transform(
       entity, asteroids::Transform(
         asteroids::Vec3(p.x(), p.y(), p.z()),
         asteroids::Vec4(o.x(), o.y(), o.z(), o.w()),
         asteroids::Vec3(pp.x(), pp.y(), pp.z())));
    auto create_command =
        asteroids::CreateCommand(fbb, 0, 0, 0, 0, 0,
                                 &update_transform);
    fbb.Finish(create_command);
    connection_component->outgoing_message_queue.Enqueue(
        fbb.Release());
  });
}

void SendRecepientFullGameState(
    asteroids::ConnectionComponent* connection, int client_id) {
  // Get all asteroid creation commands and send them
  for (const auto& asteroid :
       asteroids::GlobalGameState().asteroid_entities) {
    flatbuffers::FlatBufferBuilder fbb;
    auto command = asteroids::CreateCommand(
        fbb, 0, 0, 0, 0, &asteroid.create_asteroid);
    fbb.Finish(command);
    connection->outgoing_message_queue.Enqueue(fbb.Release());
  }
}

void ProcessNewRecipientConnections() {
  auto* connection_component =
      asteroids::GlobalGameState().singleton_components
          .Get<asteroids::ConnectionComponent>();
  auto new_recipients =
      connection_component->outgoing_message_queue.NewRecipients();
  // Send the new clients all the relevant state from the server.
  for (auto recepient : new_recipients) {
    SendRecepientFullGameState(connection_component, recepient);
  }
}

class AsteroidsServer : public game::Game {
 public:
  AsteroidsServer() : game::Game() {};
  bool Initialize() override {
    assert(!FLAGS_port.empty());
    auto* connection_component =
        asteroids::GlobalGameState().singleton_components
            .Get<asteroids::ConnectionComponent>();
    connection_component->network_thread = network::server::Create(
        FLAGS_port.c_str(),
        &connection_component->outgoing_message_queue,
        &connection_component->incoming_message_queue);
    connection_component->is_server = true;
    if (!asteroids::Initialize()) {
      std::cout << "Failed to initialize asteroids." << std::endl;
      return false;
    }
    asteroids::GlobalGameState().components
        .Assign<asteroids::GameStateComponent>(
            asteroids::GenerateFreeEntity());
    return true;
  }

  bool ProcessInput() override {
    glfwPollEvents();
    ProcessIncomingCommands();
    ProcessNewRecipientConnections();
    ProcessOutgoingCommands();
    return true;
  }

  bool Update() override {
    asteroids::UpdateGame();
    return true;
  }

  bool Render() override {
    return asteroids::RenderGame();
  }

  void OnGameEnd() override {
    auto* connection_component =
        asteroids::GlobalGameState().singleton_components
            .Get<asteroids::ConnectionComponent>();
    if (connection_component->network_thread.joinable()) {
      connection_component->incoming_message_queue.Stop();
      connection_component->network_thread.join();
    }
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  AsteroidsServer asteroids_server;
  asteroids_server.Run();
  return 0;
}
