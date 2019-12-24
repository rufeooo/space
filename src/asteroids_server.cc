#include <array>
#include <cassert>
#include <mutex>
#include <thread>
#include <vector>

#include "asteroids/asteroids.cc"
#include "asteroids/asteroids_commands.cc"
#include "asteroids/asteroids_state.cc"
#include "game/game.cc"
#include "gl/renderer.cc"
#include "math/math.cc"
#include "network/server.cc"
#include "platform/platform.cc"

static uint64_t kClientPlayers[network::server::kMaxClients];
static std::vector<int> kConnectedClients;
static std::mutex kMutex;
static const char* ip = "0.0.0.0";
static const char* port = ASTEROIDS_PORT;

void
OnClientConnected(int client_id)
{
  // Server state needs to be sent to the client on the game thread
  // so enqueue a message informing the game server to do so.
  constexpr int size =
      sizeof(asteroids::commands::ServerPlayerJoin) + game::kEventHeaderSize;
  uint8_t raw_event[size];
  asteroids::commands::ServerPlayerJoin join;
  join.client_id = client_id;
  game::Encode(sizeof(asteroids::commands::ServerPlayerJoin),
               asteroids::commands::SERVER_PLAYER_JOIN, (const uint8_t*)(&join),
               &raw_event[0]);
  game::EnqueueEvent(&raw_event[0], size);
  std::lock_guard<std::mutex> guard(kMutex);
  kConnectedClients.push_back(client_id);
}

void
OnClientMsgReceived(int client_id, uint8_t* msg, int size)
{
  game::Event e = game::Decode(msg);
  switch ((asteroids::commands::Event)e.metadata) {
    case asteroids::commands::CREATE_PLAYER: {
      // Create the the player with a server ID and update that
      // on the client.
      auto* create_player =
          game::CreateEvent<asteroids::commands::CreatePlayer>(
              asteroids::commands::CREATE_PLAYER);
      create_player->entity_id = asteroids::GenerateFreeEntity();

      // Send the client a delete and create to synchronize entity id.
      constexpr int size = sizeof(asteroids::commands::DeleteEntity) +
                           sizeof(asteroids::commands::CreatePlayer) +
                           sizeof(asteroids::commands::PlayerIdMutation) +
                           3 * game::kEventHeaderSize;

      static uint8_t data[size];
      game::EventBuilder builder;
      builder.data = data;
      builder.size = size;
      builder.idx = 0;

      // Decode incoming message to create player.
      game::Event e = game::Decode(msg);
      asteroids::commands::CreatePlayer* c =
          (asteroids::commands::CreatePlayer*)(e.data);

      // Setup responding message to delete that entity.
      asteroids::commands::DeleteEntity delete_entity;
      delete_entity.entity_id = c->entity_id;
      game::Build(sizeof(asteroids::commands::DeleteEntity),
                  asteroids::commands::DELETE_ENTITY,
                  (uint8_t*)(&delete_entity), &builder);
      game::Build(sizeof(asteroids::commands::CreatePlayer),
                  asteroids::commands::CREATE_PLAYER,
                  (uint8_t*)(&create_player[0]), &builder);
      asteroids::commands::PlayerIdMutation id_mutate;
      id_mutate.entity_id = create_player->entity_id;
      // Make sure the client updates its player id idx.
      game::Build(sizeof(asteroids::commands::PlayerIdMutation),
                  asteroids::commands::PLAYER_ID_MUTATION,
                  (uint8_t*)(&id_mutate), &builder);

      // Send message back to client.
      network::server::Send(client_id, builder.data, builder.size);

      // Save off client_id -> ship entity_id mapping.
      kClientPlayers[client_id] = create_player->entity_id;

      break;
    }
    case asteroids::commands::PLAYER_INPUT: {
      // This is ok but probably worth forcing the msg to have the right
      // player id given the client. But no cheaterz yet.
      game::EnqueueEvent(msg, size);
      break;
    }
    case asteroids::commands::CLIENT_CREATE_AUTHORITATIVE: {
      game::Event e = game::Decode(msg);
      asteroids::commands::ClientCreateAuthoritative* c =
          (asteroids::commands::ClientCreateAuthoritative*)(e.data);
      std::cout << "CLIENT CREATE CLIENT_CREATE_AUTHORITATIVE(" << c->entity_id
                << ")" << std::endl;
      break;
    }
    case asteroids::commands::CLIENT_DELETE_AUTHORITATIVE: {
      game::Event e = game::Decode(msg);
      asteroids::commands::ClientDeleteAuthoritative* c =
          (asteroids::commands::ClientDeleteAuthoritative*)(e.data);
      std::cout << "CLIENT CREATE CLIENT_DELETE_AUTHORITATIVE(" << c->entity_id
                << ")" << std::endl;
      break;
    }
    default:
      std::cout << "Unhandled client message: " << e.metadata << std::endl;
  }
}

void
SyncAuthoritativeComponents()
{
  auto& components = asteroids::GlobalGameState().components;
  std::lock_guard<std::mutex> guard(kMutex);
  if (kConnectedClients.empty()) return;
  components.Enumerate<ServerAuthoritativeComponent>(
      [&](ecs::Entity entity, ServerAuthoritativeComponent& a) {
        // TODO: Change this to EventBuilder.
        if ((a.bitmask & asteroids::commands::TRANSFORM) != 0) {
          auto* t = components.Get<TransformComponent>(entity);
          if (t) {
            constexpr int size = sizeof(asteroids::commands::UpdateTransform) +
                                 game::kEventHeaderSize;
            static uint8_t data_transform[size];
            asteroids::commands::UpdateTransform u;
            u.entity_id = entity;
            u.transform = *t;
            game::Encode(sizeof(asteroids::commands::UpdateTransform),
                         asteroids::commands::UPDATE_TRANSFORM, (uint8_t*)(&u),
                         &data_transform[0]);
            for (int client_id : kConnectedClients) {
              network::server::Send(client_id, &data_transform[0], size);
            }
          }
        }

        if ((a.bitmask & asteroids::commands::PHYSICS) != 0) {
          auto* p = components.Get<asteroids::PhysicsComponent>(entity);
          if (p) {
            constexpr int size = sizeof(asteroids::commands::UpdatePhysics) +
                                 game::kEventHeaderSize;
            static uint8_t data_physics[size];
            asteroids::commands::UpdatePhysics u;
            u.entity_id = entity;
            u.physics = *p;
            game::Encode(sizeof(asteroids::commands::UpdatePhysics),
                         asteroids::commands::UPDATE_PHYSICS, (uint8_t*)(&u),
                         &data_physics[0]);
            for (int client_id : kConnectedClients) {
              network::server::Send(client_id, &data_physics[0], size);
            }
          }
        }
      });
}

void
OptionallySendPacket(int client_id, game::EventBuilder* builder)
{
  if (builder->idx < 900) return;
  network::server::Send(client_id, builder->data, builder->idx + 1);
  builder->idx = 0;
}

void
SynchClientStateToServer(
    const asteroids::commands::ServerPlayerJoin& server_player_join)
{
  std::cout << "Syncing client: " << server_player_join.client_id << std::endl;
  auto& components = asteroids::GlobalGameState().components;

  static uint8_t data[1024];
  game::EventBuilder builder;
  builder.data = data;
  builder.size = 1024;
  builder.idx = 0;

  components.Enumerate<asteroids::AsteroidComponent, TransformComponent,
                       asteroids::RandomNumberIntChoiceComponent>(
      [&](ecs::Entity ent, asteroids::AsteroidComponent&,
          TransformComponent& transform,
          asteroids::RandomNumberIntChoiceComponent& random_num_comp) {
        asteroids::commands::CreateAsteroid create;
        create.entity_id = ent;
        create.position = transform.position;
        create.direction = math::Normalize(transform.orientation.Up());
        create.angle = transform.orientation.angle_degrees;
        create.random_number = random_num_comp.random_number;
        game::Build(sizeof(asteroids::commands::CreateAsteroid),
                    asteroids::commands::CREATE_ASTEROID,
                    (const uint8_t*)&create, &builder);
        OptionallySendPacket(server_player_join.client_id, &builder);
      });

  components.Enumerate<asteroids::ProjectileComponent, TransformComponent>(
      [&](ecs::Entity ent, asteroids::ProjectileComponent&,
          TransformComponent& transform) {
        asteroids::commands::CreateProjectile create;
        create.entity_id = ent;
        create.transform = transform;
        game::Build(sizeof(asteroids::commands::CreateProjectile),
                    asteroids::commands::CREATE_PROJECTILE,
                    (const uint8_t*)&create, &builder);
        OptionallySendPacket(server_player_join.client_id, &builder);
      });

  components.Enumerate<asteroids::PlayerComponent, TransformComponent>(
      [&](ecs::Entity ent, asteroids::PlayerComponent,
          TransformComponent& transform) {
        asteroids::commands::CreatePlayer create;
        create.entity_id = ent;
        create.position = transform.position;
        game::Build(sizeof(asteroids::commands::CreatePlayer),
                    asteroids::commands::CREATE_PLAYER, (const uint8_t*)&create,
                    &builder);
        OptionallySendPacket(server_player_join.client_id, &builder);
      });

  network::server::Send(server_player_join.client_id, builder.data,
                        builder.idx + 1);
}

void
ForwardEventToClients(game::Event event)
{
  std::lock_guard<std::mutex> guard(kMutex);
  for (int client_id : kConnectedClients) {
    network::server::Send(client_id, event.data - game::kEventHeaderSize,
                          event.size + game::kEventHeaderSize);
  }
}

void
HandleEvent(game::Event event)
{
  // Server adds events related to syncing client state.
  switch ((asteroids::commands::Event)event.metadata) {
    case asteroids::commands::SERVER_PLAYER_JOIN:
      SynchClientStateToServer(
          *((asteroids::commands::ServerPlayerJoin*)event.data));
      break;
    case asteroids::commands::CREATE_PLAYER:
    case asteroids::commands::CREATE_ASTEROID:
      ForwardEventToClients(event);
      // Fall through so the server creates the asteroid.
    default:
      asteroids::HandleEvent(event);  // Default to games HandleEvent
  };
}

bool
Initialize()
{
  network::server::Setup(&OnClientConnected, &OnClientMsgReceived);

  if (!network::server::Start(ip, port)) {
    std::cout << "Unable to start server." << std::endl;
    return 0;
  }

  if (!asteroids::Initialize()) {
    std::cout << "Failed to initialize asteroids." << std::endl;
    return false;
  }

  asteroids::GlobalGameState().components.Assign<asteroids::GameStateComponent>(
      asteroids::GenerateFreeEntity());
  return true;
}

bool
ProcessInput()
{
  PlatformEvent event;
  while (window::PollEvent(&event)) {
    int type = event.type;
    switch (type) {
      // ...
    }
  }

  return true;
}

bool
Update()
{
  SyncAuthoritativeComponents();
  asteroids::UpdateGame();
  return true;
}

bool
Render()
{
  return asteroids::RenderGame();
}

void
OnEnd()
{
  network::server::Stop();
}

// namespace

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        ip = platform_optarg;
        break;
      case 'p':
        port = platform_optarg;
        break;
    }
  }

  game::Setup(&Initialize, &ProcessInput, &HandleEvent, &Update, &Render,
              &OnEnd);

  if (!game::Run()) {
    std::cerr << "Encountered error running game..." << std::endl;
  }
  return 0;
}
